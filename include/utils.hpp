#pragma once

#include <cmath>
#include <random>

#include <mkl.h>

#include <sycl/sycl.hpp>

template <typename F>
class __scope_guard {
  F f;
public:
  __scope_guard(const F &f) : f(f) {}
  ~__scope_guard() { f(); }
};

template <typename T>
inline T generateRandom(T a = -0.5, T b = 0.5) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<T> distribution(a, b);

  return distribution(engine);
  // return 1;
}

template <typename T> T* allocDeviceAndInitAsync (
    size_t size,
    std::function<void (T *data, size_t elems)> init_func,
    sycl::queue queue
) {
  auto host_init = (T *)sycl::malloc_host(size * sizeof(T), queue);
  auto device_ptr = (T *) sycl::malloc_device(size *sizeof(T), queue);

  __scope_guard discardHost_ ([&] {
      sycl::free(host_init, queue);
  });

  for (size_t i = 0; i < size; ++ i) {
    init_func(host_init, i);
  }
  queue.memcpy(device_ptr, host_init);
  return device_ptr;
}

template <typename T1, typename T2>
static bool allClose(
    const T1 *actual, int lda, const T2 *desired, int ldb,
    const int M, const int N, const float rtol = 1e-3,
    const float atol = 1e-3
) {
  std::pair<int, int> maximum_idx;
  float maximum_err = 0.f;
  float tol = 0.f;
  for (int m = 0; m < M; ++m) {
    for (int n = 0; n < N; ++n) {
      const float a = static_cast<float>(actual[m * lda + n]);
      const float b = static_cast<float>(desired[m * ldb + n]);
      // std::cout << "actual = " << a << ", desired = " << b << std::endl;
      const float err = std::fabs(a - b);
      if (err > maximum_err) {
        maximum_idx = {m, n};
        maximum_err = err;
        tol = atol + rtol * std::fabs(b);
      }
    }
  }
  if (maximum_err > tol) {
    const int m = maximum_idx.first;
    const int n = maximum_idx.second;
    printf("Error! Matrix[%d, %d]=%.8f, ref=%.8f, error = %.8f, error term is "
           "> %E\n",
           m, n, float(actual[m * lda + n]), float(desired[m * ldb + n]),
           maximum_err, tol);
    fflush(stdout);
    return false;
  }
  return true;
}

template <typename T>
static void verifyGemm (
    const T *actual_result,
    const T *srcA, const T *srcB, int M,
    int K, int N, int lda, int ldb, int ldc
) {
  std::vector<float> a(M * K), b(K * N);
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < K; ++j) {
      a[i * K + j] = static_cast<float>(srcA[i * lda + j]);
    }
  }
  for (int i = 0; i < K; ++i) {
    for (int j = 0; j < N; ++j) {
      b[i * N + j] = static_cast<float>(srcB[i * ldb + j]);
    }
  }

  std::vector<float> expected(M * N, 0);

  cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1.0f,
              a.data(), K, b.data(), N, 0, expected.data(), N);

  bool res = allClose(actual_result, ldc, expected.data(), N, M, N);
  if (res)
    printf("test passed\n");
  else
    printf("test failed\n");
}

float timeEvent(sycl::event e) {
  return e.get_profiling_info<sycl::info::event_profiling::command_end>() -
    e.get_profiling_info<sycl::info::event_profiling::command_start>();
}
