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

template <typename T>
struct linear_copy {
  linear_copy(T* dst, T* src)
    : src(src),
    dst(dst) {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::id<1> index) const {
#if defined(__SYCL_DEVICE_ONLY__)
    T tmp, tmp2;

    lscLoad<SG_SZ, CacheCtrl::L1UC_L3UC>(tmp, (void *)(src + index));
    lscLoad<SG_SZ, CacheCtrl::L1UC_L3UC>(tmp2, (void *)(dst + index));
    lscStore<SG_SZ, CacheCtrl::L1UC_L3UC>((void *)(dst + index), tmp + tmp2);
#else
    dst[index] += src[index];
#endif
  }

  // shuffle expected
  static void verify(void* dst, void *host, T add,  size_t sz) {
    auto dst_ = reinterpret_cast<T *>(dst);
    auto cmp_ = reinterpret_cast<T *>(host);

    for (size_t i = 0; i < sz; ++ i) {
      if (dst_[i] != cmp_[i] + add) {
        std::cout<<"Verify failed: @"<<i<<","<<add<<std::endl;
        return;
      }
    }

    std::cout<<"Verify correct: "<<add<<std::endl;
  }

private:
  T* src;
  T* dst;
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("s,size", "Size of test buffer", cxxopts::value<std::string>()->default_value("32MB"))
    ;

  auto parsed_opts = opts.parse(argc, argv);
  auto sz_string = parsed_opts["size"].as<std::string>();

  auto alloc_size = parse_nelems(sz_string);

  auto queue = currentQueue(0, 0);

  auto* src = sycl::malloc_device(alloc_size, queue);
  auto* dst = sycl::malloc_device(alloc_size, queue);
  auto* b_host = sycl::malloc_host(alloc_size, queue);
  auto* b_check = sycl::malloc_host(alloc_size, queue);

  __scope_guard __guard([&]{
    sycl::free(src, queue);
    sycl::free(dst, queue);
    sycl::free(b_host, queue);
    sycl::free(b_check, queue);
  });


  fill_sequential((uint32_t *)b_host, 1, alloc_size / sizeof(uint32_t));
  queue.memcpy(src, b_host, alloc_size);
  queue.wait();

  //
  // test 32-bit, 1, 2, 4 vector;
  //
  auto nelems = alloc_size / sizeof(uint32_t);

  queue.fill<uint32_t>(dst, 5, nelems);

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> {nelems},
        linear_copy(
          reinterpret_cast<uint32_t *>(dst),
          reinterpret_cast<uint32_t *>(src)));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  linear_copy<uint32_t>::verify(b_check, b_host, 5, nelems);

  queue.fill<uint32_t>(dst, 4, nelems);

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> {nelems / 2},
        linear_copy(
          reinterpret_cast<sycl::vec<uint32_t, 2> *>(dst),
          reinterpret_cast<sycl::vec<uint32_t, 2> *>(src)));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();
  linear_copy<uint32_t>::verify(b_check, b_host, 4, nelems);

  queue.fill<uint32_t>(dst, 3, nelems);

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> {nelems / 4},
        linear_copy(
          reinterpret_cast<sycl::vec<uint32_t, 4> *>(dst),
          reinterpret_cast<sycl::vec<uint32_t, 4> *>(src)));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();
  linear_copy<uint32_t>::verify(b_check, b_host, 3, nelems);

  // -------------------------------------------------------------
  // this test is not in target
  /*
  nelems = alloc_size / sizeof(sycl::half);
  fill_sequential((sycl::half *)b_host, 1, alloc_size / sizeof(sycl::half));

  queue.memcpy(src, b_host, alloc_size);
  queue.fill<sycl::half>(dst, 3.1, nelems);
  queue.wait(); // flush the cache, because we are ready to use non-cache load/store

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> {nelems},
        linear_copy(
          reinterpret_cast<sycl::half *>(dst),
          reinterpret_cast<sycl::half *>(src)));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();
  linear_copy<sycl::half>::verify(b_check, b_host, 3.1, nelems);
  */

  queue.fill<sycl::half>(dst, 2.5, nelems);
  queue.wait(); // flush the cache, because we are ready to use non-cache load/store

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> {nelems / 2},
        linear_copy(
          reinterpret_cast<sycl::vec<sycl::half, 2> *>(dst),
          reinterpret_cast<sycl::vec<sycl::half, 2> *>(src)));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();
  linear_copy<sycl::half>::verify(b_check, b_host, 2.5, nelems);

  queue.fill<sycl::half>(dst, 1.3, nelems);
  queue.wait(); // flush the cache, because we are ready to use non-cache load/store

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> {nelems / 8},
        linear_copy(
          reinterpret_cast<sycl::vec<sycl::half, 8> *>(dst),
          reinterpret_cast<sycl::vec<sycl::half, 8> *>(src)));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();
  linear_copy<sycl::half>::verify(b_check, b_host, 1.3, nelems);
}
