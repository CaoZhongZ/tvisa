#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "gen_visa_templates.hpp"

#define SG_SZ 16

size_t parse_nelems(const std::string& nelems_string) {
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

template <typename T>
void fill_sequential(T *p, int rank, size_t nelems) {
  for (size_t i = 0; i < nelems; ++ i) {
    p[i] = i + rank;
  }
}

template <int N, typename T>
struct tile_accumulate {
  tile_accumulate(T* dst, T* src) : src(src), dst(dst) {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::id<1> index) const {
#if defined(__SYCL_DEVICE_ONLY__)
    constexpr int M = sizeof(int) / sizeof(T);

    // occupy 64-byte register
    union region {
      sycl::vec<T, M*N> block;
      sycl::vec<T, M> elem[N];
    };
    region tmp1;
    region tmp2;

    constexpr int pseudo_pitch = 64;
    constexpr int sg_stride = N * pseudo_pitch;

    lscLoad<SG_SZ, DataShuffle::none, CacheCtrl::L1UC_L3UC>(
        tmp1.elem, (void *)(src + index * sg_stride), pseudo_pitch, N, pseudo_pitch, 0, 0);
    lscLoad<SG_SZ, DataShuffle::none, CacheCtrl::L1UC_L3UC>(
        tmp2.elem, (void *)(src + index * sg_stride), pseudo_pitch, N, pseudo_pitch, 0, 0);

    region ret;
#   pragma unroll
    for (int i = 0; i < N; ++ i)
      ret.elem[i] = tmp1.elem[i] + tmp2.elem[i];

    lscStore<SG_SZ, DataShuffle::none, CacheCtrl::L1UC_L3UC>(
        (void *)(dst + index * sg_stride), ret.elem, pseudo_pitch, N, pseudo_pitch, 0, 0);
#else
    dst[index] += src[index];
#endif
  }

private:
  T* src;
  T* dst;
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("s,size", "Size of test buffer",
     cxxopts::value<std::string>()->default_value("32MB"))
    ;

  auto parsed_opts = opts.parse(argc, argv);
  auto sz_string = parsed_opts["size"].as<std::string>();

  auto alloc_size = parse_nelems(sz_string);

  auto queue = currentQueue(0, 0);

  auto* src = sycl::malloc_device(alloc_size, queue);
  auto* dst = sycl::malloc_device(alloc_size, queue);
  auto* b_host = sycl::malloc_host(alloc_size, queue);
  auto* b_check = sycl::malloc_host(alloc_size, queue);

  release_guard __guard([&]{
    sycl::free(src, queue);
    sycl::free(dst, queue);
    sycl::free(b_host, queue);
    sycl::free(b_check, queue);
  });

  fill_sequential((sycl::half *)b_host, 1, alloc_size / sizeof(sycl::half));
  queue.memcpy(src, b_host, alloc_size);
  queue.wait();

  constexpr int ROW = 8;

  auto nelems = alloc_size / sizeof(sycl::half);

  queue.fill<sycl::half>(dst, 1.2, nelems);

  auto block_sz = ROW * SG_SZ * sizeof(uint32_t);
  auto blocks = (alloc_size + block_sz - 1) / block_sz;

  //  ------------SG_SZ * 4-byte--------------
  //  |                                      |
  // Row              Block                  |
  //  |                                      |
  //  ----------------------------------------

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> { blocks },
        tile_accumulate<ROW, sycl::half>(
          reinterpret_cast<sycl::half *>(dst),
          reinterpret_cast<sycl::half *>(src)));
  });
}
