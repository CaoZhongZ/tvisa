#include "cxxopts.hpp"
#include "gen_visa_templates.hpp"
#include "lsc.hpp"
#include "sycl_misc.hpp"
#include "utils.hpp"
#include <csignal>
#include <iostream>
#include <sycl/sycl.hpp>

#define SG_SZ 16

#ifdef __SYCL_DEVICE_ONLY__
#define __SYCL_CONSTANT_AS __attribute__((opencl_constant))
#else
#define __SYCL_CONSTANT_AS
#endif

const __SYCL_CONSTANT_AS char fmt[] = " %f";
const __SYCL_CONSTANT_AS char fmt_end[] = "\n";

size_t parse_nelems(const std::string &nelems_string) {
  size_t base = 1;
  size_t pos = nelems_string.rfind("K");
  if (pos != std::string::npos) {
    base = 1024ull;
  } else {
    pos = nelems_string.rfind("M");
    if (pos != std::string::npos)
      base = 1024 * 1024ull;
    else {
      pos = nelems_string.rfind("G");
      if (pos != std::string::npos)
        base = 1024 * 1024 * 1024ull;
    }
  }

  return stoull(nelems_string) * base;
}

template <typename T> void fill_sequential(T *p, int rank, size_t nelems) {
  for (size_t i = 0; i < nelems; ++i) {
    p[i] = i + rank;
  }
}

template <int Height, int Width, typename T> struct tile_accumulate {
  tile_accumulate(T *dst, T *src, int surfaceH, int surfaceW, int surfaceP)
      : src(src), dst(dst), surfaceH(surfaceH), surfaceW(surfaceW),
        surfaceP(surfaceP) {}

  void operator()
      [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
    auto index = item.get_global_linear_id();
#if defined(__SYCL_DEVICE_ONLY__)
    int x_off = 0;
    int y_off = index / SG_SZ * Height;

    __Matrix<T, Height, Width, DataShuffle::none, SG_SZ> tmp0;
    __Matrix<T, Height, Width, DataShuffle::none, SG_SZ> tmp1;
    AddressPayload<Height, Width> address_payload_0(
        (void *)src, surfaceH, surfaceW, surfaceP, x_off, y_off);
    AddressPayload<Height, Width> address_payload_1(
        (void *)dst, surfaceH, surfaceW, surfaceP, x_off, y_off);
    lscLoad(tmp0, address_payload_0);
    lscLoad(tmp1, address_payload_1);

    __ArrayMatrix<T, Height, Width, DataShuffle::none, SG_SZ> ret = tmp0 + tmp1;
    lscStore<CacheCtrl::L1WB_L3WB>(address_payload_1, ret);
#else
    dst[index] += src[index];
#endif
  }

private:
  T *src;
  T *dst;
  int surfaceH;
  int surfaceW;
  int surfaceP;
};

template <int BlockM, int BlockN, typename T> struct vnni_test {
  vnni_test(T *dst, T *src, int surfaceH, int surfaceW, int surfaceP)
      : src(src), dst(dst), surfaceH(surfaceH), surfaceW(surfaceW),
        surfaceP(surfaceP) {}

  void operator()
      [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
    auto index = item.get_global_linear_id();
#if defined(__SYCL_DEVICE_ONLY__)
    int x_off = 0;
    int y_off = 0;

    __ArrayMatrix<T, BlockN, BlockM, DataShuffle::none, SG_SZ> tmp0;
    AddressPayload<BlockN, BlockM> address_payload_0(
        (void *)src, 
        surfaceH, 
        surfaceW * sizeof(T), 
        surfaceP * sizeof(T), 
        x_off, y_off);
    lscLoad<CacheCtrl::L1UC_L3UC>(tmp0, address_payload_0);

    AddressPayload<BlockN, BlockM> address_payload_1(
        (void *)dst, 
        surfaceH, 
        surfaceW * sizeof(T), 
        surfaceP * sizeof(T), x_off, y_off);
    lscStore<CacheCtrl::L1WB_L3WB>(address_payload_1, tmp0);
#else
    dst[index] = src[index];
#endif
  }

private:
  T *src;
  T *dst;
  int surfaceH;
  int surfaceW;
  int surfaceP;
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()("s,size", "Size of test buffer",
                     cxxopts::value<std::string>()->default_value("32MB"))(
      "h,height", "Height of the surface",
      cxxopts::value<int>()->default_value("1024"))(
      "w,width", "Width of the surface",
      cxxopts::value<int>()->default_value("64"))(
      "p,pitch", "Pitch of the surface",
      cxxopts::value<int>()->default_value("64"));

  auto parsed_opts = opts.parse(argc, argv);
  auto sz_string = parsed_opts["size"].as<std::string>();

  auto surfaceH = parsed_opts["height"].as<int>();
  auto surfaceW = parsed_opts["width"].as<int>();
  auto surfaceP = parsed_opts["pitch"].as<int>();
  auto alloc_size = surfaceH * surfaceP * sizeof(sycl::half);

  sycl::queue queue(
      sycl::gpu_selector_v,
      sycl::property_list{sycl::property::queue::enable_profiling(),
                          sycl::property::queue::in_order()});

  auto *src = sycl::malloc_device(alloc_size, queue);
  auto *dst = sycl::malloc_device(alloc_size, queue);
  auto *b_host = sycl::malloc_host(alloc_size, queue);
  auto *b_check = sycl::malloc_host(alloc_size, queue);

  __scope_guard __guard([&] {
    sycl::free(src, queue);
    sycl::free(dst, queue);
    sycl::free(b_host, queue);
    sycl::free(b_check, queue);
  });
  typedef sycl::half InT;

  fill_sequential((InT *)b_host, 0., alloc_size / sizeof(InT));
  queue.memcpy(src, b_host, alloc_size);
  queue.wait();

  auto nelems = alloc_size / sizeof(InT);

  queue.fill<InT>(dst, 1, nelems);

  int tensorH = surfaceH;
  constexpr int tensorW = 16; // bytes
  constexpr int tensorP = tensorW;
  constexpr int BlockM = 16;
  constexpr int BlockN = 16;

  //  ------------SG_SZ * 4-byte--------------
  //  |                                      |
  // Row              Block                  |
  //  |                                      |
  //  ----------------------------------------

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::nd_range<1>{SG_SZ, SG_SZ},
                   vnni_test<BlockM, BlockN, InT>(reinterpret_cast<InT *>(dst),
                                                  reinterpret_cast<InT *>(src),
                                                  surfaceH, surfaceW,
                                                  surfaceP));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  std::cout << "----------------------------------" << std::endl;
  for (int k = 0; k < BlockN; ++k) {
    for (int i = 0; i < BlockM; ++i)
      std::cout << ((InT *)b_check)[k * surfaceP + i] << ", ";
    std::cout << std::endl;
  }
  std::cout << "----------------------------------" << std::endl;
}
