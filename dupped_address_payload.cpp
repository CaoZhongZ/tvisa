#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"

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
  tile_accumulate(T* dst, T* src, int surfaceH, int surfaceW, int surfaceP)
    : src(src), dst(dst), surfaceH(surfaceH), surfaceW(surfaceW), surfaceP(surfaceP)
  {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::id<1> index) const {
#if defined(__SYCL_DEVICE_ONLY__)
    int M = sizeof(int) / sizeof(T);
    int pseudo_pitch = surfaceP -1;
    int x_off = 0;
    int y_off = index/SG_SZ * N;
    int surface_height = surfaceH -1;
    int surface_width = surfaceW -1;

    asm volatile (".decl Varray0 v_type=G type=d num_elts=64 align=wordx32\n");
    asm volatile (".decl Varray2 v_type=G type=d num_elts=64 align=wordx32\n");

    asm volatile (".decl Sep0 v_type=G type=f num_elts=16 align=wordx32 alias=<Varray0, 0>\n");
    asm volatile (".decl Sep1 v_type=G type=f num_elts=16 align=wordx32 alias=<Varray0, 64>\n");
    asm volatile (".decl Sep2 v_type=G type=f num_elts=16 align=wordx32 alias=<Varray0, 128>\n");
    asm volatile (".decl Sep3 v_type=G type=f num_elts=16 align=wordx32 alias=<Varray0, 192>\n");
    asm volatile (".decl Sep4 v_type=G type=f num_elts=16 align=wordx32 alias=<Varray2, 0>\n");
    asm volatile (".decl Sep5 v_type=G type=f num_elts=16 align=wordx32 alias=<Varray2, 64>\n");
    asm volatile (".decl Sep6 v_type=G type=f num_elts=16 align=wordx32 alias=<Varray2, 128>\n");
    asm volatile (".decl Sep7 v_type=G type=f num_elts=16 align=wordx32 alias=<Varray2, 192>\n");

    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1_NM, 1) Varray0:d32.16x8nn flat[%0, %1, %2, %3, %4, %5]\n"
        "lsc_load_block2d.ugm (M1_NM, 1) Varray2:d32.16x8nn flat[%6, %1, %2, %3, %4, %5]\n"
        "add (M1, 16) Sep0(0, 0)<1> Sep0(0, 0)<1;1,0> Sep4(0, 0)<1;1,0>\n"
        "add (M1, 16) Sep1(0, 0)<1> Sep1(0, 0)<1;1,0> Sep5(0, 0)<1;1,0>\n"
        "add (M1, 16) Sep2(0, 0)<1> Sep2(0, 0)<1;1,0> Sep6(0, 0)<1;1,0>\n"
        "add (M1, 16) Sep3(0, 0)<1> Sep3(0, 0)<1;1,0> Sep7(0, 0)<1;1,0>\n"
        "lsc_store_block2d.ugm (M1_NM, 1) flat[%6, %1, %2, %3, %4, %5] Varray0:d32.16x4nn\n"
        :: "rw"(src), "rw"(surface_width), "rw"(surface_height), "rw"(pseudo_pitch),
        "rw"(x_off), "rw"(y_off), "rw"(dst));
#else
    dst[index] += src[index];
#endif
  }

private:
  T* src;
  T* dst;
  int surfaceH;
  int surfaceW;
  int surfaceP;
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("s,size", "Size of test buffer",
     cxxopts::value<std::string>()->default_value("512"))
    ("h,height", "Height of the surface",
     cxxopts::value<int>()->default_value("1024"))
    ("w,width", "Width of the surface",
     cxxopts::value<int>()->default_value("64"))
    ("p,pitch", "Pitch of the surface",
     cxxopts::value<int>()->default_value("64"))
    ;

  auto parsed_opts = opts.parse(argc, argv);
  auto sz_string = parsed_opts["size"].as<std::string>();

  auto alloc_size = parse_nelems(sz_string);
  auto surfaceH = parsed_opts["height"].as<int>();
  auto surfaceW = parsed_opts["width"].as<int>();
  auto surfaceP = parsed_opts["pitch"].as<int>();

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

  fill_sequential((float *)b_host, 0., alloc_size / sizeof(float));

  queue.memcpy(src, b_host, alloc_size);
  queue.wait();

  constexpr int ROW = 8;

  auto nelems = alloc_size / sizeof(float);

  queue.fill<float>(dst, 5, nelems);

  auto block_sz = ROW * SG_SZ * sizeof(uint32_t);
  auto blocks = (alloc_size + block_sz - 1) / block_sz;

  //  ------------SG_SZ * 4-byte--------------
  //  |                                      |
  // Row              Block                  |
  //  |                                      |
  //  ----------------------------------------

  std::cout<<"Num. of block: "<<blocks<<std::endl;
  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> { blocks * SG_SZ },
        tile_accumulate<ROW, float>(
          reinterpret_cast<float *>(dst),
          reinterpret_cast<float *>(src),
          surfaceH, surfaceW, surfaceP));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  std::cout<<"----------------------------------"<<std::endl;

  for (int k = 0; k < 8; ++ k) {
    for (int i = 0; i < 64/sizeof(float); ++ i)
      std::cout<<((float *)b_check)[k*64/sizeof(float) + i]<<", ";
    std::cout<<std::endl;
  }

  std::cout<<"----------------------------------"<<std::endl;

  for (int k = 8; k < 16; ++ k) {
    for (int i = 0; i < 64/sizeof(float); ++ i)
      std::cout<<((float *)b_check)[k*64/sizeof(float) + i]<<", ";
    std::cout<<std::endl;
  }
}
