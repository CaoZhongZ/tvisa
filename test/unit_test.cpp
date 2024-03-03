#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "utils.hpp"
#include "gen_visa_templates.hpp"

#include "test_submatrix_cast.hpp"

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
void show_tile(T* start, int W, int H, size_t pitch) {
  auto* tile = reinterpret_cast<T (*)[pitch/sizeof(T)]>(start);
  for (int h = 0; h < H; ++ h) {
    for (int w = 0; w < W; ++ w) {
      std::cout<<tile[h][w]<<", ";
    }
    std::cout<<std::endl;
  }
}

int main(int argc, char *argv[]) {
  cxxopts::Options opts("unit", "Unit test");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("p,pitch", "Pitch of the surface",
     cxxopts::value<size_t>()->default_value("4096"))
    ("l,length", "Length of the outer surface",
     cxxopts::value<size_t>()->default_value("4096"))
    ("w,width", "Width of the surface",
     cxxopts::value<int>()->default_value("1024"))
    ("h,height", "Height of the surface",
     cxxopts::value<int>()->default_value("1024"))
    ("g,groups", "Number of subgroups",
     cxxopts::value<size_t>()->default_value("1"))
    ;

  auto parsed_opts = opts.parse(argc, argv);

  auto surfaceP = parsed_opts["pitch"].as<size_t>();
  auto surfaceL = parsed_opts["length"].as<size_t>();
  auto surfaceH = parsed_opts["height"].as<int>();
  auto surfaceW = parsed_opts["width"].as<int>();

  auto groups = parsed_opts["groups"].as<size_t>();
  auto nelems = surfaceP * surfaceL;

  using t_type = sycl::half;
  auto alloc_size = nelems * sizeof(t_type);

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

  fill_sequential(
      (t_type *)b_host, 0., nelems
  );

  queue.memcpy(src, b_host, alloc_size);
  queue.fill<t_type>(dst, 8., nelems);
  queue.wait();

  constexpr int Width = 16;
  constexpr int Height = 8;
  constexpr int SG_SZ = 16;

  queue.submit([&](sycl::handler &h) {
      h.parallel_for(
        sycl::nd_range<2> (sycl::range<2>(1, SG_SZ), sycl::range<2>(1, SG_SZ)),
        tileSplitAdd<t_type, Height, Width, SG_SZ>(
          (t_type *)dst, (t_type *)src, surfaceH, surfaceW, surfaceP
        )
      );
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  queue.submit([&](sycl::handler &h) {
      h.parallel_for(
        sycl::nd_range<2> (sycl::range<2>(1, SG_SZ), sycl::range<2>(1, SG_SZ)),
        tileCastAdd<uint32_t, t_type, Height, Width, SG_SZ>(
          (uint32_t *)dst, (uint32_t *)src, surfaceH, surfaceW, surfaceP
        )
      );
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  queue.submit([&](sycl::handler &h) {
      h.parallel_for(
        sycl::nd_range<2> (sycl::range<2>(1, SG_SZ), sycl::range<2>(1, SG_SZ)),
        tileCastAdd<t_type, float, Height, Width, SG_SZ>(
          (t_type *)dst, (t_type *)src, surfaceH, surfaceW, surfaceP
        )
      );
  });
}
