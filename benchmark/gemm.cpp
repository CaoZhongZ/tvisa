#include <cmath>
#include <mkl.h>
#include <random>
#include <iostream>
#include <iomanip>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "gen_visa_templates.hpp"
#include "mma.hpp"
#include "sycl_misc.hpp"
#include "utils.hpp"

template <typename T, int SubGroupSize = 16> struct gemmKernel {
  gemmKernel(
      uint32_t M, uint32_t N, uint32_t K,
      const T *A, uint32_t lda,
      const T *B, uint32_t ldb,
      T *C,  uint32_t ldc
  ) : M(M), N(N), K(K),
  pitchA(lda * sizeof(T)), pitchB(ldb * sizeof(T)), pitchC(ldc * sizeof(T)),
  A(A), B(B), C(C) {}

#if defined(__SYCL_DEVICE_ONLY__)
  inline void rawSampleSubGroupGemm(int subStartM, int subStartN) const {
    constexpr int mElems = 16;
    constexpr int nElems = 16 * 2;
    constexpr int kElems = 16;

    auto pos = sycl::ext::oneapi::experimental::this_nd_item<2>();
    auto sg_Y = pos.get_local_id()[0];
    auto sg_X = pos.get_local_id()[1] / SubGroupSize;

    AddressPayload<16, 16> addressA_0(
        A, M, K * sizeof(T), pitchA, 0, subStartM
    );
    AddressPayload<16, 16, 2> addressB_0(
        B, K, N * sizeof(T), pitchB, subStartN, 0
    );

    AddressPayload<16, 16> addressA_1(addressA_0);
    AddressPayload<16, 16, 2> addressB_1(addressB_0);

    // Balance prefetch among sub-groups
    AddressPayload<32/4, 16> addressPrefetch_A(addressA_0);
    AddressPayload<16/4, 64/2> addressPrefetch_B(addressB_0);

    addressA_1.addSrc0AddrY(mElems);
    addressB_1.addSrc0AddrX(nElems);

    // Initial slab position
    addressPrefetch_A.addSrc0AddrY(sg_X * 32/4);
    addressPrefetch_B.addSrc0AddrX((sg_Y % 2) * 64/2);
    addressPrefetch_B.addSrc0AddrY((sg_Y / 2) * 16/4);

    for (int i = 0; i < 3; ++ i) {
      addressPrefetch_A.prefetch<T, SubGroupSize, CacheCtrl::L1C_L3C>();
      addressPrefetch_B.prefetch<T, SubGroupSize, CacheCtrl::L1C_L3C>();

      addressPrefetch_A.addSrc0AddrX(kElems);
      addressPrefetch_B.addSrc0AddrY(kElems);
    }

    using mTA = __RawMatrix<T, 16, 16, DataShuffle::none, SubGroupSize>;
    using mTB = __RawMatrix<T, 16, 16, DataShuffle::vnni, SubGroupSize, 2>;
    using mTC = __ArrayMatrix<T, 16, 16, DataShuffle::none, SubGroupSize>;

    // Systolic march
    mTA A_0, A_1;//  mTA A_C0, A_C1;
    mTB B_0, B_1;//  mTB B_C0, B_C1;

    mTC C_00, C_01, C_02, C_03;
    mTC C_10, C_11, C_12, C_13;

    memset(&C_00, 0, sizeof(mTC));
    memset(&C_01, 0, sizeof(mTC));
    memset(&C_02, 0, sizeof(mTC));
    memset(&C_03, 0, sizeof(mTC));
    memset(&C_10, 0, sizeof(mTC));
    memset(&C_11, 0, sizeof(mTC));
    memset(&C_12, 0, sizeof(mTC));
    memset(&C_13, 0, sizeof(mTC));

    for (int k = 0; k < K; k += kElems) {
      B_0.load(addressB_0);
      B_1.load(addressB_1);
      A_0.load(addressA_0);
      A_1.load(addressA_1);
      addressPrefetch_B.prefetch<T, SubGroupSize, CacheCtrl::L1C_L3C>();
      addressPrefetch_A.prefetch<T, SubGroupSize, CacheCtrl::L1C_L3C>();

      addressB_0.addSrc0AddrY(kElems);
      addressB_1.addSrc0AddrY(kElems);
      addressA_0.addSrc0AddrX(kElems);

      swFence();
      dpas<SubGroupSize>(C_00, A_0, B_0.template subArrayView<0>());
      dpas<SubGroupSize>(C_10, A_1, B_0.template subArrayView<0>());

      dpas<SubGroupSize>(C_01, A_0, B_0.template subArrayView<1>());
      dpas<SubGroupSize>(C_11, A_1, B_0.template subArrayView<1>());

      dpas<SubGroupSize>(C_02, A_0, B_1.template subArrayView<0>());
      dpas<SubGroupSize>(C_12, A_1, B_1.template subArrayView<0>());

      dpas<SubGroupSize>(C_03, A_0, B_1.template subArrayView<1>());
      dpas<SubGroupSize>(C_13, A_1, B_1.template subArrayView<1>());
      swFence();

      addressA_1.addSrc0AddrX(kElems);
      addressPrefetch_B.addSrc0AddrY(kElems);
      addressPrefetch_A.addSrc0AddrX(kElems);
   }

    AddressPayload<8, 32> address_C(
        C, M, N * sizeof(T), pitchC, subStartN, subStartM
    );

    auto C_merge00 = concat(
        C_00.template subTileView<0, 8>(), C_01.template subTileView<0, 8>()
    );
    C_merge00.store(address_C);

    auto C_merge01 = concat(
        C_02.template subTileView<0, 8>(), C_03.template subTileView<0, 8>()
    );
    address_C.addSrc0AddrX(32);
    C_merge01.store(address_C);

    auto C_merge11 = concat(
        C_02.template subTileView<8, 8>(), C_03.template subTileView<8, 8>()
    );
    address_C.addSrc0AddrY(8);
    C_merge11.store(address_C);

    auto C_merge10 = concat(
        C_00.template subTileView<8, 8>(), C_01.template subTileView<8, 8>()
    );
    address_C.addSrc0AddrX(-32);
    C_merge10.store(address_C);

    auto C_merge20 = concat(
        C_10.template subTileView<0, 8>(), C_11.template subTileView<0, 8>()
    );
    address_C.addSrc0AddrY(8);
    C_merge20.store(address_C);

    auto C_merge21 = concat(
        C_12.template subTileView<0, 8>(), C_13.template subTileView<0, 8>()
    );
    address_C.addSrc0AddrX(32);
    C_merge21.store(address_C);

    auto C_merge31 = concat(
        C_12.template subTileView<8, 8>(), C_13.template subTileView<8, 8>()
    );
    address_C.addSrc0AddrY(8);
    C_merge31.store(address_C);

    auto C_merge30 = concat(
        C_10.template subTileView<8, 8>(), C_11.template subTileView<8, 8>()
    );
    address_C.addSrc0AddrX(-32);
    C_merge30.store(address_C);
  }

  inline void groupGemm(int groupStartM, int groupStartN) const {
    auto pos = sycl::ext::oneapi::experimental::this_nd_item<2>();
    auto sub_M = pos.get_local_id()[0];
    auto sub_N = pos.get_local_id()[1]/SubGroupSize;

    int subYElems = 32;
    int subXElems = 64;

    auto subStartM = groupStartM + subYElems * sub_M;
    auto subStartN = groupStartN + subXElems * sub_N;

    rawSampleSubGroupGemm(subStartM, subStartN);
  }

  inline void topGemm(int globalStartM, int globalStartN) const {
    auto pos = sycl::ext::oneapi::experimental::this_nd_item<2>();
    auto gid_M = pos.get_group().get_group_id()[0];
    auto gid_N = pos.get_group().get_group_id()[1];

    int groupYElems = pos.get_local_range()[0] * 32;
    int groupXElems = pos.get_local_range()[1]/SubGroupSize * 64;

    auto groupStartM = globalStartM + groupYElems * gid_M;
    auto groupStartN = globalStartN + groupXElems * gid_N;

    groupGemm(groupStartM, groupStartN);
  }
#endif

  void operator()[[sycl::reqd_sub_group_size(SubGroupSize)]] (
      sycl::nd_item<2> pos
  ) const {
    int gpuYElems = pos.get_group_range()[0] * pos.get_local_range()[0] * 32;
    int gpuXElems = pos.get_group_range()[1] * pos.get_local_range()[1]/SubGroupSize * 64;

#if defined(__SYCL_DEVICE_ONLY__)
    for (int n = 0; n < N; n += gpuXElems)
      for (int m = 0; m < M; m += gpuYElems)
        topGemm(m, n);
#endif
  }

  static sycl::event launch(
      sycl::queue queue, sycl::nd_range<2> launch_param,
      int32_t M, uint32_t N, uint32_t K,
      T* A, uint32_t lda, T* B, uint32_t ldb, T* C, uint32_t ldc
  ) {
    return queue.submit(
      [&](sycl::handler &h) {
        h.parallel_for(launch_param, gemmKernel(M, N, K, A, lda, B, ldb, C, ldc));
    });
  }
private:
  int M, N, K;
  // GPU convention
  int pitchA, pitchB, pitchC;

  const T *A;
  const T *B;
  T *C;
};

template <typename T> void dumpMatrix(T* C, int M, int N, int ldc) {
  for (int i = 0; i < M; ++ i) {
    for (int j = 0; j < N; ++ j)
      std::cout<<C[i * ldc + j]<<", ";
    std::cout<<std::endl;
  }
}

int main(int argc, char *argv[]) {
  cxxopts::Options opts("gemm", "Systolic GEMM test");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("M,M_", "M dimension", cxxopts::value<int>()->default_value("4096"))
    ("K,K_", "K dimension", cxxopts::value<int>()->default_value("4096"))
    ("N,N_", "N dimension", cxxopts::value<int>()->default_value("4096"))
    ("a,lda", "Leading dimension of A", cxxopts::value<int>()->default_value("-1"))
    ("b,ldb", "Leading dimension of B", cxxopts::value<int>()->default_value("-1"))
    ("c,ldc", "Leading dimension of C", cxxopts::value<int>()->default_value("-1"))
    ("m,groupM", "Groups alone M dimention to launch", cxxopts::value<size_t>()->default_value("16"))
    ("n,groupN", "Groups alone N dimention to launch", cxxopts::value<size_t>()->default_value("16"))
    ("l,subGroupM", "Sub-Groups alone M dimention to launch", cxxopts::value<size_t>()->default_value("8"))
    ("w,subGroupN", "Sub-Groups alone N dimention to launch", cxxopts::value<size_t>()->default_value("4"))
    ;

  auto parsed_opts = opts.parse(argc, argv);

  auto M = parsed_opts["M_"].as<int>();
  auto K = parsed_opts["K_"].as<int>();
  auto N = parsed_opts["N_"].as<int>();

  typedef sycl::half testType;

  auto lda = parsed_opts["lda"].as<int>();
  auto ldb = parsed_opts["ldb"].as<int>();
  auto ldc = parsed_opts["ldc"].as<int>();

  if (lda < 0) lda = K;
  if (ldb < 0) ldb = N;
  if (ldc < 0) ldc = N;

  auto nGroupM = parsed_opts["m"].as<size_t>();
  auto nGroupN = parsed_opts["n"].as<size_t>();
  auto nSubGroupM = parsed_opts["l"].as<size_t>();
  auto nSubGroupN = parsed_opts["w"].as<size_t>();

  sycl::queue queue = currentQueue(0, 0);

  auto elemsA = M * K;

  testType *A_host, *A, *B_host, *B, *C_host, *C;

  std::tie(A_host, A) = allocDeviceAndInitAsync<testType>(
      M * K, [](testType* a, size_t i) {
        a[i] = static_cast<testType>(generateRandom<float>());
        // a[i] = static_cast<testType>((float)i);
      }, queue);

  std::tie(B_host, B) = allocDeviceAndInitAsync<testType>(
      K * N, [](testType* a, size_t i) {
        a[i] = static_cast<testType>(generateRandom<float>());
        // a[i] = static_cast<testType>((float)i);
      }, queue);

  std::tie(C_host, C) = allocDeviceAndInitAsync<testType>(
      M * N, [](testType* a, size_t i) {
        a[i] = 0;
      }, queue);

  __scope_guard __guard([&] {
    sycl::free(A, queue);
    sycl::free(B, queue);
    sycl::free(C, queue);

    sycl::free(A_host, queue);
    sycl::free(B_host, queue);
    sycl::free(C_host, queue);
  });

  constexpr int SG_SZ = 16;

  sycl::range<2> local(nSubGroupM, nSubGroupN * SG_SZ);
  sycl::range<2> global(nGroupM, nGroupN);
  sycl::nd_range<2> param (global * local, local);

  gemmKernel<testType, SG_SZ>::launch(
      queue, param, M, N, K, A, lda, B, ldb, C, ldc);

  queue.memcpy(C_host, C, sizeof(testType) * M * N).wait();
  verifyGemm(C_host, A_host, B_host, M, N, K, lda, ldb, ldc);
#if defined(DEBUG)
  dumpMatrix(C_host, M, N, ldc);
#endif

  constexpr const int iter = 20;
  float durations = 0.f;

  gemmKernel<testType, SG_SZ>::launch(
      queue, param, M, N, K, A, lda, B, ldb, C, ldc
  );

  for (int i = 0; i < iter; ++i) {
    auto e = gemmKernel<testType, SG_SZ>::launch(
        queue, param, M, N, K, A, lda, B, ldb, C, ldc
    );
    durations += timeEvent(e);
  }

  durations /= iter;

  float ops = size_t(M) * K * N * 2;
  float Tflops = ops / 1e3 / durations;
  printf("M: %d, N: %d, K: %d, time: %f us, TFlops: %f\n", M, N, K, durations / 1e3, Tflops);
}
