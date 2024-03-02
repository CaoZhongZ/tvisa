#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "gen_visa_templates.hpp"

#define SG_SZ 16

#define xstr(s) str(s)
#define str(x) #x

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

void fill_sequential(uint64_t *p, int rank, size_t nelems) {
  for (size_t i = 0; i < nelems; ++ i) {
    p[i] = i + rank;
  }
}

union merge {
  uint64_t whole;
  sycl::vec<uint32_t, 2> split;
};

struct linear_copy {
  linear_copy(sycl::vec<uint32_t, 2>* dst, uint64_t* src)
    : src(src), dst(dst) {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::id<1> index) const {
#if defined(__SYCL_DEVICE_ONLY__)
    merge tmp;

    lscLoad<SG_SZ>(tmp.whole, (void *)(src + index));
    lscStore<SG_SZ>((void *)(dst + index), tmp.split);
#else
    merge tmp;
    tmp.whole = src[index];
    dst[index] = tmp.split;
#endif
  }

  // shuffle expected
  static void verify(uint32_t* dst, uint32_t *host, size_t nelems) {
    auto shuffle = reinterpret_cast<uint32_t (*)[2][SG_SZ]>(host);

    for (int i = 0; i < nelems; ++ i) {
      if (shuffle[i /(SG_SZ * 2)][i % 2][i / 2] != dst[i])
        std::cout<<"Verify failed @"<<i<<std::endl;
    }
  }

private:
  uint64_t* src;
  sycl::vec<uint32_t, 2>* dst;
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("n,nelems", "Number of elements", cxxopts::value<std::string>()->default_value("16MB"))
    ;

  auto parsed_opts = opts.parse(argc, argv);
  auto nelems_string = parsed_opts["nelems"].as<std::string>();

  auto nelems = parse_nelems(nelems_string);
  using test_type = uint64_t;
  size_t alloc_size = nelems * sizeof(test_type);

  auto queue = currentQueue(0, 0);

  auto* src = (test_type *)sycl::malloc_device(alloc_size, queue);
  auto* dst = (sycl::vec<uint32_t, 2> *)sycl::malloc_device(alloc_size, queue);
  auto* b_host = sycl::malloc_host(alloc_size, queue);
  auto* b_check = sycl::malloc_host(alloc_size, queue);

  __scope_guard __guard([&]{
    sycl::free(src, queue);
    sycl::free(dst, queue);
    sycl::free(b_host, queue);
    sycl::free(b_check, queue);
  });

  fill_sequential((uint64_t *)b_host, 1, nelems);

  queue.memcpy(src, b_host, alloc_size);
  queue.wait();

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> {nelems}, linear_copy(dst, src));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  linear_copy::verify((uint32_t *)b_check, (uint32_t *)b_host, alloc_size);
}
