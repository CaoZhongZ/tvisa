#include <csignal>
#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "gen_visa_templates.hpp"
#include "lsc.hpp"
#include "sycl_misc.hpp"

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

template <int Height, int Width, typename T> struct vnni_test {
  vnni_test(T *dst, T *src, int surfaceH, int surfaceW, int surfaceP)
      : src(src), dst(dst), surfaceH(surfaceH), surfaceW(surfaceW),
        surfaceP(surfaceP) {}

  void operator()
      [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
    auto index = item.get_global_linear_id();
#if defined(__SYCL_DEVICE_ONLY__)
    int x_off = 0;
    int y_off = index / SG_SZ * Height;

    __Matrix<T, Height, Width, DataShuffle::none, SG_SZ> tmp0;
    AddressPayload<Height, Width> address_payload_0(
        (void *)src, surfaceH, surfaceW, surfaceP, x_off, y_off);
    lscLoad<CacheCtrl::L1UC_L3UC>(tmp0, address_payload_0);

    __Matrix<T, Height, Width, DataShuffle::none, SG_SZ> ret;
    sycl::vec<T, __Matrix<T, Height, Width, DataShuffle::none, SG_SZ>::N>
        image = tmp0.getImage();
    ret.getImage() = image;

    if (index == 0) {
      constexpr int N = __Matrix<T, Height, Width, DataShuffle::none, SG_SZ>::N;
      for (int i = 0; i < N; ++i) {
        sycl::ext::oneapi::experimental::printf(fmt, float(image[i]));
      }
      sycl::ext::oneapi::experimental::printf(fmt_end);
    }

    AddressPayload<Height, Width> address_payload_1(
        (void *)dst, surfaceH, surfaceW, surfaceP, x_off, y_off);
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

  auto alloc_size = parse_nelems(sz_string);
  auto surfaceH = parsed_opts["height"].as<int>();
  auto surfaceW = parsed_opts["width"].as<int>();
  auto surfaceP = parsed_opts["pitch"].as<int>();

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
  constexpr int tensorW = 32; // bytes
  constexpr int tensorP = tensorW;
  constexpr int ROW = 32;
  constexpr int COL = tensorW / sizeof(InT);
  auto block_sz = ROW * COL * sizeof(InT);
  auto blocks = (alloc_size + block_sz - 1) / block_sz;

  //  ------------SG_SZ * 4-byte--------------
  //  |                                      |
  // Row              Block                  |
  //  |                                      |
  //  ----------------------------------------

  std::cout << "Num. of block: " << blocks << std::endl;
  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::nd_range<1>{blocks * SG_SZ, SG_SZ},
                   vnni_test<ROW, COL, InT>(reinterpret_cast<InT *>(dst),
                                            reinterpret_cast<InT *>(src),
                                            tensorH, tensorW, tensorP));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  std::cout << "----------------------------------" << std::endl;

  for (int k = 0; k < ROW; ++k) {
    for (int i = 0; i < COL; ++i)
      std::cout << ((InT *)b_check)[k * tensorP / sizeof(InT) + i] << ", ";
    std::cout << std::endl;
  }

  std::cout << "----------------------------------" << std::endl;

  for (int k = ROW; k < ROW * 2; ++k) {
    for (int i = 0; i < COL; ++i)
      std::cout << ((InT *)b_check)[k * tensorP / sizeof(InT) + i] << ", ";
    std::cout << std::endl;
  }
}
