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
    p[i] = i/1024. + rank;
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

template <int W, int H, typename T>
struct tile_accumulate {
  tile_accumulate(T* dst, T* src, int surfaceW, int surfaceH, int surfaceP)
    : src(src), dst(dst), surfaceH(surfaceH), surfaceW(surfaceW), surfaceP(surfaceP)
  {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::id<1> index) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto grp_num = index / SG_SZ;

    auto x_off = grp_num * 16;
    auto y_off = 0;

    AddressPayload<W, H> srcAddress(src, surfaceW, surfaceH, surfaceP, x_off, y_off);
    AddressPayload<W, H> dstAddress(srcAddress);

    dstAddress.updateSurfaceBase(dst);

    __Matrix<T, W, H> tmp0;
    __Matrix<T, W, H> tmp1;

    lscLoad<CacheCtrl::L1UC_L3UC>(tmp0, srcAddress);
    lscLoad<CacheCtrl::L1UC_L3UC>(tmp1, dstAddress);

    /*
    tmp0.load(srcAddress);
    tmp1.load(dstAddress);
    */

    auto ret = tmp0 + tmp1;
    lscStore<CacheCtrl::L1UC_L3UC>(dstAddress, ret);

    // ret.store(dstAddress);

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
    ("p,pitch", "Pitch of the surface",
     cxxopts::value<size_t>()->default_value("4096"))
    ("s,surround", "Height of the outer surface",
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
  auto surround = parsed_opts["surround"].as<size_t>();
  auto surfaceH = parsed_opts["height"].as<int>();
  auto surfaceW = parsed_opts["width"].as<int>();

  auto groups = parsed_opts["groups"].as<size_t>();

  using t_type = sycl::half;

  auto nelems = surfaceP * surround / sizeof(t_type);
  auto alloc_size = surfaceP * surround;

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
  queue.wait();

  constexpr int Width = 16;
  constexpr int Height = 8;

  queue.fill<t_type>(dst, 8., nelems);

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> { groups * SG_SZ },
        tile_accumulate<Width, Height, t_type>(
          reinterpret_cast<t_type *>(dst),
          reinterpret_cast<t_type *>(src),
          surfaceW, surfaceH, surfaceP));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();

  show_tile((t_type *)b_host, Width, Height, surfaceP);
  std::cout<<"------------------------------------------------"<<std::endl;
  show_tile((t_type *)b_check, Width, Height, surfaceP);
}
