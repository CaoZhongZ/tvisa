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

template <typename T>
void fill_sequential(T *p, T rank, size_t nelems) {
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

union unpacked {
  uint32_t full;
  sycl::half mid;
};

struct dump_half {
  dump_half(uint32_t* dst, sycl::half* src1, sycl::half* src2)
    : src1(src1), src2(src2), dst(dst) {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::id<1> index) const {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        ".decl Vsrc1HF v_type=G type=hf num_elts=32 align=wordx32\n"
        ".decl Vsrc2HF v_type=G type=hf num_elts=32 align=wordx32\n"
        ".decl VdstHF v_type=G type=hf num_elts=32 align=wordx32\n"
        "lsc_load.ugm (M1, 16) Vsrc1HF:d16c32 flat[%0]:a64\n"
        "lsc_load.ugm (M1, 16) Vsrc2HF:d16c32 flat[%1]:a64\n"
        "add (M1, 16) VdstHF(0, 0)<1> Vsrc1HF(0, 0)<1;1,0> Vsrc2HF(0, 0)<1;1,0>\n"
        "lsc_store.ugm (M1, 16) flat[%2]:a64 VdstHF:d32\n"
        :: "rw"(src1 + index), "rw"(src2 + index), "rw"(dst + index));
#else
    unpacked p;
    p.mid = src1[index];
    dst[index] = p.full;
#endif
  }

private:
  sycl::half* src1;
  sycl::half* src2;
  uint32_t* dst;
};


int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("s,size", "Number of elements", cxxopts::value<std::string>()->default_value("32MB"))
    ;

  auto parsed_opts = opts.parse(argc, argv);
  auto size_string = parsed_opts["size"].as<std::string>();

  auto size = parse_nelems(size_string);
  size_t alloc_size = size;

  auto queue = currentQueue(0, 0);

  auto* src1 = sycl::malloc_device(alloc_size, queue);
  auto* src2 = sycl::malloc_device(alloc_size, queue);
  auto* dst = sycl::malloc_device(alloc_size, queue);
  auto* b_host = sycl::malloc_host(alloc_size, queue);
  auto* b_check = sycl::malloc_host(alloc_size, queue);

  release_guard __guard([&]{
    sycl::free(src1, queue);
    sycl::free(src2, queue);
    sycl::free(dst, queue);
    sycl::free(b_host, queue);
    sycl::free(b_check, queue);
  });

  auto nelems = alloc_size / sizeof(uint32_t);
  fill_sequential((sycl::half *)b_host, (sycl::half)1, nelems);

  queue.memcpy(src1, b_host, alloc_size);
  queue.memcpy(src2, b_host, alloc_size);
  queue.wait();

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> {nelems}, dump_half((uint32_t *)dst,(sycl::half*)src1, (sycl::half*)src2));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  for (int j = 0; j < 4; ++ j) {
    for (int i = 0; i < 32; ++ i)
      std::cout<<((sycl::half *)b_check)[j * 32 + i]<<" ";
    std::cout<<std::endl;
  }
}
