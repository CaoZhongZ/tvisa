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


struct tile_accumulate {
  tile_accumulate(float* dst, float* src, size_t nelems, int surfaceH, int surfaceW, int surfaceP)
    : src(src), dst(dst), nelems(nelems), surfaceH(surfaceH), surfaceW(surfaceW), surfaceP(surfaceP)
  {}

  inline void accum_2d(float* dst, float* src, size_t block) const {
#if defined(__SYCL_DEVICE_ONLY__)
    int pseudo_pitch = surfaceP -1;
    int x_off = 0;
    int y_off = block * 4;
    int surface_height = surfaceH -1;
    int surface_width = surfaceW -1;

    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1_NM, 1) Varray0:d32.16x4nn flat[%0, %1, %2, %3, %4, %5]\n"
        "lsc_load_block2d.ugm (M1_NM, 1) Varray2:d32.16x4nn flat[%6, %1, %2, %3, %4, %5]\n"
        "add (M1, 16) Sep0(0, 0)<1> Sep0(0, 0)<1;1,0> Sep4(0, 0)<1;1,0>\n"
        "add (M1, 16) Sep1(0, 0)<1> Sep1(0, 0)<1;1,0> Sep5(0, 0)<1;1,0>\n"
        "add (M1, 16) Sep2(0, 0)<1> Sep2(0, 0)<1;1,0> Sep6(0, 0)<1;1,0>\n"
        "add (M1, 16) Sep3(0, 0)<1> Sep3(0, 0)<1;1,0> Sep7(0, 0)<1;1,0>\n"
        "lsc_store_block2d.ugm (M1_NM, 1) flat[%6, %1, %2, %3, %4, %5] Varray0:d32.16x4nn\n"
        :: "rw"(src), "rw"(surface_width), "rw"(surface_height), "rw"(pseudo_pitch),
        "rw"(x_off), "rw"(y_off), "rw"(dst));
#endif
  }

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> pos) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto n_blocks = nelems / 64;

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

    auto group_id = pos.get_group().get_group_id()[0];
    auto n_group = pos.get_group().get_group_range()[0];

    auto n_sub_group = pos.get_sub_group().get_group_range()[0];
    auto sub_group_id = pos.get_sub_group().get_group_id()[0];

    auto global_sg_id = group_id * n_sub_group + sub_group_id;

    for (auto block_id = global_sg_id; block_id < n_blocks; block_id += n_group)
      accum_2d(dst, src, block_id);
#else
    (void)src, (void)dst, (void)nelems, (void)surfaceH, (void)surfaceW, (void)surfaceP;
#endif
  }

private:
  float* src;
  float* dst;
  size_t nelems;

  int surfaceH;
  int surfaceW;
  int surfaceP;
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("s,size", "Size of test buffer",
     cxxopts::value<std::string>()->default_value("4096"))
    ("h,height", "Height of the surface",
     cxxopts::value<int>()->default_value("4096"))
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

  auto nelems = alloc_size / sizeof(float);

  queue.fill<float>(dst, 5, nelems);

  //  ------------SG_SZ * 4-byte--------------
  //  |                                      |
  // Row              Block                  |
  //  |                                      |
  //  ----------------------------------------

  // for simplicity, we use single group including single sub-group
  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::nd_range<1> { sycl::range<1>{16}, sycl::range<1>{16} },
        tile_accumulate(
          reinterpret_cast<float *>(dst),
          reinterpret_cast<float *>(src),
          nelems, surfaceH, surfaceW, surfaceP));
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
