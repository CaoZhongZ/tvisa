#include "cxxopts.hpp"
#include "gen_visa_templates.hpp"
#include "mma.hpp"
#include "sycl_misc.hpp"
#include <cmath>
#include <mkl.h>
#include <random>
#include <sycl/sycl.hpp>

constexpr const int sg_size = 16;
constexpr const int sg_tile_m = 32;
constexpr const int sg_tile_n = 64;
constexpr const int wg_tile_m = 256;
constexpr const int wg_tile_n = 256;
constexpr const int wg_size_m = wg_tile_m / sg_tile_m;
constexpr const int wg_size_n = wg_tile_n / sg_tile_n;
constexpr const int k_stride = 16;
constexpr const int k_unroll = 0;

constexpr const int prefetch_stage = 3;

// Large regsiter file dictate 32 sub-group per SS
template <typename T, int SubGroupSize = 16> struct groupGemmKernel {
  groupGemmKernel(
      const T *A, uint32_t lda,
      const T *B, uint32_t ldb,
      T *C,  uint32_t ldc,
      uint32_t M, uint32_t K, uint32_t N
  ) : A(A), B(B), C(C), lda(lda), ldb(ldb), ldc(ldc), M(M), K(K), N(N) {}

  static inline rawSampleSubGroupGemm(int mGroupStart, int nGroupStart) const {
    auto pos = sycl::ext::oneapi::experimental::this_nd_item<3>();
    int subGroupM = pos.get_local_id()[0];
    int subGroupN = pos.get_local_id()[1];

    int subMOff = mGroupStart + subGroupM * 32 * sizeof(T);
    int subNOff = nGroupStart + subGroupN * 64 * sizeof(T);

    constexpr int strideVSize = 16 * sizeof(T);
    constexpr int strideHSize = 16 * 2 * sizeof(T);
    constexpr int strideDElems = 16;

    AddressPayload<16, 16> addressA_0(A, M, K, lda, 0, subMOff);
    AddressPayload<16, 16> addressA_1(addressA_0).addSrc0AddrY(strideVSize);
    AddressPayload<16, 16, 2> addressB_0(B, K, N, ldb, subNOff, 0);
    AddressPayload<16, 16, 2> addressB_1(B_0).addSrc0AddrX(strideHSize);

    using mTA = __ArrayMatrix<T, 16, 16, DataShuffle::none, SubGroupSize>;
    using mTB = __ArrayMatrix<T, 16, 16, DataShuffle::vnni, SubGroupSize, 2>;
    using mTC = __ArrayMatrix<T, 16, 16, DataShuffle::none, SubGroupSize>;

    // Systolic march
    mTA A_0, A_1;
    mTB B_0, B_1;
    mTC C_00, C_01, C_10, C_11;

    for (int k = 0; k < K; k += strideDElems) {
    }

    AddressPayload<8, 16> addressC(C, M, N, ldc, subMOff, subNOff);
  }

  static inline groupGemm() const {
    auto pos = sycl::ext::oneapi::experimental::this_nd_item<3>();

    auto subGroupM = pos.get_local_id()[0];
    auto subGroupN = pos.get_local_id()[1];

    auto groupM = pos.get_group().get_group_id()[0];
    auto groupN = pos.get_group().get_gropu_id()[1];

  }

  void operator()[[sycl::reqd_sub_group_size(SubGroupSize)]] (
      sycl::nd_item<3> item
  ) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto sg_id_m = item.get_local_id(0);
    auto sg_id_n = item.get_local_id(1);
    auto wg_id_m = item.get_group(0);
    auto wg_id_n = item.get_group(1);

    int m_offset = wg_id_m * wg_tile_m + sg_id_m * sg_tile_m;
    int n_offset = wg_id_n * wg_tile_n + sg_id_n * sg_tile_n;
    int k_loop = matrix_k / k_stride;

    constexpr const int mma_m = MMA32x32x16::mma_m_in_elem;
    constexpr const int mma_n = MMA32x32x16::mma_n_in_elem;
    constexpr const int mma_k = MMA32x32x16::mma_k_in_bytes / sizeof(T);

    constexpr const int segment_m = sg_tile_m / mma_m;
    constexpr const int segment_n = sg_tile_n / mma_n;
    constexpr const int segment_k = k_stride / mma_k;

    __ArrayMatrix<T, mma_m, mma_n, DataShuffle::none, sg_size> acc[4];

    AddressPayload<mma_m, mma_k> a_address(
        (void *)A, (uint32_t)matrix_m,
        static_cast<uint32_t>(matrix_k * sizeof(T)),
        static_cast<uint32_t>(matrix_k * sizeof(T)), 0, m_offset);
    AddressPayload<mma_k, mma_n> b_address(
        (void *)B, (uint32_t)matrix_k,
        static_cast<uint32_t>(matrix_n * sizeof(T)),
        static_cast<uint32_t>(matrix_n * sizeof(T)), n_offset, 0);
    AddressPayload<mma_m, mma_n> c_address(
        (void *)C, (uint32_t)matrix_m,
        static_cast<uint32_t>(matrix_n * sizeof(T)),
        static_cast<uint32_t>(matrix_n * sizeof(T)), n_offset, m_offset);

    // AddressPayload<mma_m, mma_k>  prefetch_a_address(a_address);
    // AddressPayload<mma_k, mma_n> prefetch_b_address(b_address);
    // load A
    __ArrayMatrix<T, mma_m, mma_k, DataShuffle::none, sg_size> mat_a;
    mat_a.load(a_address);

    // load B
    __ArrayMatrix<T, mma_k, mma_n, DataShuffle::vnni, sg_size> mat_b0;
    __ArrayMatrix<T, mma_k, mma_n, DataShuffle::vnni, sg_size> mat_b1;
    __ArrayMatrix<T, mma_k, mma_n, DataShuffle::vnni, sg_size> mat_b2;
    __ArrayMatrix<T, mma_k, mma_n, DataShuffle::vnni, sg_size> mat_b3;

    lscLoad<CacheCtrl::L1C_L3C>(mat_b0, b_address);
    b_address.addSrc0AddrX(mma_n);
    lscLoad<CacheCtrl::L1C_L3C>(mat_b1, b_address);
    b_address.addSrc0AddrX(mma_n);
    lscLoad<CacheCtrl::L1C_L3C>(mat_b2, b_address);
    b_address.addSrc0AddrX(mma_n);
    lscLoad<CacheCtrl::L1C_L3C>(mat_b3, b_address);

#   pragma unroll
    for (int i = 0; i < 4; ++ i) {
      acc[i].load(c_address);
      c_address.addSrc0AddrX(mma_n);
    }

    asm("fence_sw");

    for (int k = 0; k < k_loop; ++k) {

      // if constexpr (prefetch_stage != 0) {
      //   lscPrefetch<CacheCtrl::L1C_L3C, T, sg_tile_m / wg_size_n, mma_k,
      //               DataShuffle::none, sg_size>(prefetch_a_address);
      //   lscPrefetch<CacheCtrl::L1C_L3C, T, mma_k, sg_tile_n / wg_size_m,
      //               DataShuffle::none, sg_size>(prefetch_b_address);
      // }

      asm("fence_sw");
      // mma
      dpas(acc[0], mat_a, mat_b0);
      dpas(acc[1], mat_a, mat_b1);
      dpas(acc[2], mat_a, mat_b2);
      dpas(acc[3], mat_a, mat_b3);

      asm("fence_sw");
      // if constexpr (prefetch_stage != 0) {
      //   prefetch_a_address.addSrc0AddrX();
      //   prefetch_b_address.addSrc0AddrY();
      // }
    }

    // store c

    c_address.addSrc0AddrY(-mma_n * 3);
    lscStore<CacheCtrl::L1WB_L3WB>(c_address, acc[0]);
    c_address.addSrc0AddrY(mma_n);
    lscStore<CacheCtrl::L1WB_L3WB>(c_address, acc[1]);
    c_address.addSrc0AddrY(mma_n);
    lscStore<CacheCtrl::L1WB_L3WB>(c_address, acc[2]);
    c_address.addSrc0AddrY(mma_n);
    lscStore<CacheCtrl::L1WB_L3WB>(c_address, acc[3]);

#else
    auto id = item.get_global_linear_id();
    C[id] = A[id] + B[id];
#endif
  }

  const T *A;
  const T *B;
  T *C;

  // 2D load basically can only address inside of 24-bit
  uint32_t lda, ldb, ldc;
  uint32_t M, K, N;
};

template <typename T>
sycl::event compute_kernel(sycl::queue &q, const T *A, const T *B, T *C,
                           const int matrix_m, const int matrix_k,
                           const int matrix_n) {
  auto num_wg_m = matrix_m / wg_tile_m;
  auto num_wg_n = matrix_n / wg_tile_n;

  sycl::range<3> local_range(wg_size_m, wg_size_n, sg_size);
  sycl::range<3> global_range(num_wg_m, num_wg_n, 1);
  MMAKernelImpl<T> task(A, B, C, matrix_m, matrix_k, matrix_n);

  auto evt = q.submit([&](sycl::handler &cgh) {
    cgh.parallel_for(
        sycl::nd_range<3>(global_range * local_range, local_range),
        task
    );
  });
  return evt;
}

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()("m", "m of mma",
                     cxxopts::value<int>()->default_value("4096"))(
      "k", "k of mma", cxxopts::value<int>()->default_value("4096"))(
      "n", "n of mma", cxxopts::value<int>()->default_value("4096"));

  auto parsed_opts = opts.parse(argc, argv);
  auto matrix_m = parsed_opts["m"].as<int>();
  auto matrix_n = parsed_opts["n"].as<int>();
  auto matrix_k = parsed_opts["k"].as<int>();

  typedef sycl::half InT;
  uint32_t size_a = matrix_m * matrix_k;
  uint32_t size_b = matrix_k * matrix_n;
  uint32_t size_c = matrix_m * matrix_n;

  using data_type_a = InT;
  using data_type_b = InT;
  using data_type_c = InT;

  sycl::queue queue = getQueue<0, 0>();

  auto context = queue.get_info<sycl::info::queue::context>();
  auto device = queue.get_info<sycl::info::queue::device>();

  std::cout << "Device: " << device.get_info<sycl::info::device::name>()
            << ", Platform: "
            << device.get_platform().get_info<sycl::info::platform::name>()
            << std::endl;

  auto A = alloc_shared_and_init<data_type_a>(
      size_a,
      [](data_type_a *data, size_t idx) {
        data[idx] = static_cast<data_type_a>(random_float());
      },
      queue, device, context);
  auto B = alloc_shared_and_init<data_type_b>(
      size_b,
      [](data_type_b *data, size_t idx) {
        data[idx] = static_cast<data_type_b>(random_float());
      },
      queue, device, context);
  auto C = alloc_shared_and_init<data_type_c>(
      size_c,
      [](data_type_c *data, size_t idx) {
        data[idx] = static_cast<data_type_c>(0.0f);
      },
      queue, device, context);

  scope_guard __guard([&] {
    sycl::free(A, queue);
    sycl::free(B, queue);
    sycl::free(C, queue);
  });

  // warm up
  compute_kernel(queue, A, B, C, matrix_m, matrix_k, matrix_n);

  constexpr const int iter = 20;
  float durations(0);
  for (int i = 0; i < iter; ++i) {
    auto evt = compute_kernel(queue, A, B, C, matrix_m, matrix_k, matrix_n);
    durations += get_gpu_time_from_event(evt);
  }
  durations /= iter;

  verify_result(C, A, B, matrix_m, matrix_k, matrix_n, matrix_k, matrix_n,
                matrix_n);

  float ops = size_t(matrix_m) * matrix_k * matrix_n * 2;
  float Tflops = ops / 1e3 / durations;
  printf("M: %d, N: %d, K: %d, time: %f us, TFlops: %f\n", matrix_m, matrix_n,
         matrix_k, durations / 1e3, Tflops);
}
