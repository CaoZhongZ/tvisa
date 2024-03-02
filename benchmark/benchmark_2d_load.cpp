#include "cxxopts.hpp"
#include "event.hpp"
#include "gen_visa_templates.hpp"
#include "sycl_misc.hpp"
#include <cmath>
#include <sycl/sycl.hpp>

constexpr const int sg_size = 16;
constexpr const int sg_tile_m = 32;
constexpr const int sg_tile_n = 64;
constexpr const int wg_tile_m = 128;
constexpr const int wg_tile_n = 256;
constexpr const int wg_size_m = wg_tile_m / sg_tile_m;
constexpr const int wg_size_n = wg_tile_n / sg_tile_n;

bool all_close(float a, float b, float rtol = 1e-5, float atol = 1e-5) {
  const float err = std::fabs(a - b);
  if (err > atol + rtol * std::fabs(b))
    return false;
  return true;
}

float get_gpu_time_from_event(sycl::event &gpu_event) {
  auto gpu_start =
      gpu_event
          .get_profiling_info<sycl::info::event_profiling::command_start>();
  auto gpu_end =
      gpu_event.get_profiling_info<sycl::info::event_profiling::command_end>();
  float gpu_time = (gpu_end - gpu_start);
  return gpu_time; // ns
}

template <typename T>
void verify_result(const T *actual, const T *expected, int surfaceW,
                   int surfaceH) {
  int err = 0;
  for (int m = 0; m < surfaceH; ++m) {
    for (int n = 0; n < surfaceW; ++n) {
      if (!all_close(actual[m * surfaceW + n], expected[m * surfaceW + n]))
        ++err;
      if (err && err < 3)
        printf("Error! Matrix[%d, %d]=%.8f, ref=%.8f\n", m, n,
               float(actual[m * surfaceW + n]),
               float(expected[m * surfaceW + n]));
    }
  }
  if (!err)
    printf("Test passed\n");
  else
    printf("Test failed, failed number : %d\n", err);
}

template <typename T> void fill_sequential(T *p, int rank, size_t nelems) {
  for (size_t i = 0; i < nelems; ++i) {
    p[i] = i + rank;
  }
}

template <typename T> struct CopyKernelImpl {
  CopyKernelImpl(const T *src, T *dst, int surfaceW, int surfaceH)
      : src(src), dst(dst), surfaceW(surfaceW), surfaceH(surfaceH) {}
  [[sycl::reqd_sub_group_size(sg_size)]] void
  operator()(sycl::nd_item<3> item) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto sg_id_m = item.get_local_id(0);
    auto sg_id_n = item.get_local_id(1);
    auto wg_id_m = item.get_group(0);
    auto wg_id_n = item.get_group(1);

    int m_offset = wg_id_m * wg_tile_m + sg_id_m * sg_tile_m;
    int n_offset = wg_id_n * wg_tile_n + sg_id_n * sg_tile_n;

    constexpr const int b_m = 32;
    constexpr const int b_n = 16;
    constexpr const int loop_m = sg_tile_m / b_m;
    constexpr const int loop_n = sg_tile_n / b_n;
    AddressPayload<b_m, b_n> src_address(
        (void *)src, (uint32_t)surfaceH,
        static_cast<uint32_t>(surfaceW * sizeof(T)),
        static_cast<uint32_t>(surfaceW * sizeof(T)), n_offset, m_offset);
    __ArrayMatrix<T, b_m, b_n, DataShuffle::none, sg_size> data[loop_m][loop_n];
    for (int ih = 0; ih < loop_m; ++ih) {
      AddressPayload<b_m, b_n> src_block_address(src_address);
      src_block_address.addSrc0AddrY(ih * b_m);

      lscLoad<CacheCtrl::L1C_L3C>(data[ih][0], src_block_address);
      for (int iw = 1; iw < loop_n; ++iw) {
        src_block_address.addSrc0AddrX(b_n);
        lscLoad<CacheCtrl::L1C_L3C>(data[ih][iw], src_block_address);
      }
    }

    AddressPayload<b_m, b_n> dst_address(
        (void *)dst, (uint32_t)surfaceH,
        static_cast<uint32_t>(surfaceW * sizeof(T)),
        static_cast<uint32_t>(surfaceW * sizeof(T)), n_offset, m_offset);
    for (int ih = 0; ih < loop_m; ++ih) {
      AddressPayload<b_m, b_n> dst_block_address(dst_address);
      dst_block_address.addSrc0AddrY(ih * b_m);

      lscStore<CacheCtrl::L1WB_L3WB>(dst_block_address, data[ih][0]);
      for (int iw = 1; iw < loop_n; ++iw) {
        dst_block_address.addSrc0AddrX(b_n);
        lscStore<CacheCtrl::L1WB_L3WB>(dst_block_address, data[ih][iw]);
      }
    }
#else
    auto id = item.get_global_linear_id();
    dst[id] = src[id];
#endif
  }
  const T *src;
  T *dst;
  int surfaceW;
  int surfaceH;
};

template <typename T>
sycl::event copy_kernel(sycl::queue &q, const T *src, T *dst, int surfaceW,
                        int surfaceH) {
  auto num_wg_m = surfaceH / wg_tile_m;
  auto num_wg_n = surfaceW / wg_tile_n;

  sycl::range<3> local_range(wg_size_m, wg_size_n, sg_size);
  sycl::range<3> global_range(num_wg_m, num_wg_n, 1);

  auto evt = q.submit([&](sycl::handler &cgh) {
    CopyKernelImpl<T> task(src, dst, surfaceW, surfaceH);
    cgh.parallel_for(sycl::nd_range<3>(global_range * local_range, local_range),
                     task);
  });
  return evt;
}

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()("h,height", "Height of the surface",
                     cxxopts::value<int>()->default_value("16384"))(
      "w,pitch", "Width of the surface",
      cxxopts::value<int>()->default_value("8192"));

  auto parsed_opts = opts.parse(argc, argv);
  auto surfaceH = parsed_opts["height"].as<int>();
  auto surfaceP = parsed_opts["pitch"].as<int>();

  typedef sycl::half InT;
  auto surfaceW = surfaceP / sizeof(InT);
  auto alloc_elems = surfaceH * surfaceW;

  sycl::queue queue(
      sycl::gpu_selector_v,
      sycl::property_list{sycl::property::queue::enable_profiling(),
                          sycl::property::queue::in_order()});

  auto *src = sycl::malloc_device<InT>(alloc_elems, queue);
  auto *dst = sycl::malloc_device<InT>(alloc_elems, queue);
  auto *b_host = sycl::malloc_host<InT>(alloc_elems, queue);
  auto *b_check = sycl::malloc_host<InT>(alloc_elems, queue);

  __scope_guard __guard([&] {
    sycl::free(src, queue);
    sycl::free(dst, queue);
    sycl::free(b_host, queue);
    sycl::free(b_check, queue);
  });

  fill_sequential(b_host, 0., alloc_elems);
  queue.memcpy(src, b_host, alloc_elems * sizeof(InT));
  queue.wait();

  // warm up
  copy_kernel(queue, src, dst, surfaceW, surfaceH);

  constexpr const int iter = 10;
  float durations(0);
  for (int i = 0; i < iter; ++i) {
    auto evt = copy_kernel(queue, src, dst, surfaceW, surfaceH);
    durations += get_gpu_time_from_event(evt);
  }
  durations /= iter;
  queue.memcpy(b_check, dst, alloc_elems * sizeof(InT));

  verify_result(b_check, b_host, surfaceW, surfaceH);

  float io = alloc_elems * sizeof(InT) * 2;
  float bw = alloc_elems * sizeof(InT) * 2 / durations;
  printf("io: %f M, bandwidth: %f GB/s\n", io / 1e6, bw);
}
