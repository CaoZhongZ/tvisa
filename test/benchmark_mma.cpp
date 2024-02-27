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
constexpr const int k_unroll = 8;

constexpr const int prefetch_stage = 0;

template <typename result_type>
inline result_type generate_random(result_type a = -0.5, result_type b = 0.5) {
  unsigned seed =
  std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<result_type> distribution(a, b);

  return distribution(engine);
  // return 1;
}

#define random_float() (generate_random<double>())

template <typename data_type>
inline data_type *alloc_shared_and_init(
    size_t size,
    std::function<void(data_type *data, size_t elements)> init_func,
    sycl::queue &queue, sycl::device &device, sycl::context &context) {
  auto device_ptr = static_cast<data_type *>(
      aligned_alloc_shared(4096, size * sizeof(data_type), device, context));

  for (size_t i = 0; i < size; ++i) {
    init_func(device_ptr, i);
  }

  return device_ptr;
}

template <typename T1, typename T2>
bool all_close(const T1 *actual, int lda, const T2 *desired, int ldb,
               const int M, const int N, const float rtol = 1e-3,
               const float atol = 1e-3) {
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
void verify_result(const T *actual_result, const T *srcA, const T *srcB, int M,
                   int K, int N, int lda, int ldb, int ldc) {
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

  // cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1.0f,
  //             a.data(), K, b.data(), N, 0, expected.data(), N);

  bool res = all_close(actual_result, ldc, expected.data(), N, M, N);
  if (res)
    printf("test passed\n");
  else
    printf("test failed\n");
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

template <typename T> struct MMAKernelImpl {
  MMAKernelImpl(const T *A, const T *B, T *C, const int matrix_m,
                const int matrix_k, const int matrix_n)
      : A(A), B(B), C(C), matrix_m(matrix_m), matrix_k(matrix_k),
        matrix_n(matrix_n) {}
  [[sycl::reqd_sub_group_size(sg_size)]] void
  operator()(sycl::nd_item<3> item) const {
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
    __ArrayMatrix<T, mma_m, mma_n, DataShuffle::none, sg_size> acc[segment_m]
                                                                  [segment_n];
    for (int im = 0; im < segment_m; ++im) {
      for (int in = 0; in < segment_n; ++in) {
        acc[im][in].zero();
      }
    }

    AddressPayload<mma_m, mma_k> a_address(
        (void *)A, (uint32_t)matrix_m,
        static_cast<uint32_t>(matrix_k * sizeof(T)),
        static_cast<uint32_t>(matrix_k * sizeof(T)), 0, m_offset);
    AddressPayload<mma_k, mma_n> b_address(
        (void *)B, (uint32_t)matrix_k,
        static_cast<uint32_t>(matrix_n * sizeof(T)),
        static_cast<uint32_t>(matrix_n * sizeof(T)), n_offset, 0);

    AddressPayload<mma_m, mma_k> prefetch_a_address(a_address);
    AddressPayload<mma_k, mma_n> prefetch_b_address(b_address);
    for (int i = 0; i < prefetch_stage; ++i) {

      int pre_k_start = (sg_id_m + sg_id_n + i) % k_unroll;
      prefetch_a_address.updateSrc0AddrX(k_stride * pre_k_start);
      prefetch_b_address.updateSrc0AddrY(k_stride * pre_k_start);               
      
      // prefetch a
      for (int im = 0; im < segment_m; ++im) {
        AddressPayload<mma_m, mma_k> prefetch_a_block_address(
            prefetch_a_address);
        prefetch_a_block_address.addSrc0AddrY(im * mma_m);

        lscPrefetch<CacheCtrl::L1C_L3C, T, mma_m, mma_k, DataShuffle::none,
                    sg_size>(prefetch_a_block_address);
        for (int ik = 1; ik < segment_k; ++ik) {
          prefetch_a_block_address.addSrc0AddrX(mma_k);
          lscPrefetch<CacheCtrl::L1C_L3C, T, mma_m, mma_k, DataShuffle::none,
                      sg_size>(prefetch_a_block_address);
        }
      }

      // prefetch b
      for (int ik = 0; ik < segment_k; ++ik) {
        AddressPayload<mma_k, mma_n> prefetch_b_block_address(
            prefetch_b_address);
        prefetch_b_block_address.addSrc0AddrY(ik * mma_k);

        lscPrefetch<CacheCtrl::L1C_L3C, T, mma_k, mma_n, DataShuffle::vnni,
                sg_size>(prefetch_b_block_address);
        for (int in = 1; in < segment_n; ++in) {
          prefetch_b_block_address.addSrc0AddrX(mma_n);
          lscPrefetch<CacheCtrl::L1C_L3C, T, mma_k, mma_n, DataShuffle::vnni,
                  sg_size>(prefetch_b_block_address);
        }
      }
      prefetch_a_address.addSrc0AddrX(k_stride);
      prefetch_b_address.addSrc0AddrY(k_stride);
    }

    for (int k = 0; k < k_loop; k+= k_unroll) {
      for(int kk=0; kk<k_unroll; ++kk) {
      int k_start = (sg_id_m + sg_id_n + k + kk) % k_unroll;
      a_address.updateSrc0AddrX(k_stride * (k + k_start));
      b_address.updateSrc0AddrY(k_stride * (k + k_start));      
      
      if constexpr (prefetch_stage != 0){
      int pre_k_start = (sg_id_m + sg_id_n + k + kk + prefetch_stage) % k_unroll;      
      prefetch_a_address.updateSrc0AddrX(k_stride * pre_k_start);
      prefetch_b_address.updateSrc0AddrY(k_stride * pre_k_start);        
      }
      
      // load A
      __ArrayMatrix<T, mma_m, mma_k, DataShuffle::none, sg_size>
          mat_a[segment_m][segment_k];
      for (int im = 0; im < segment_m; ++im) {
        AddressPayload<mma_m, mma_k> a_block_address(a_address);
        a_block_address.addSrc0AddrY(im * mma_m);

        lscLoad<CacheCtrl::L1C_L3C>(mat_a[im][0], a_block_address);
        for (int ik = 1; ik < segment_k; ++ik) {
          a_block_address.addSrc0AddrX(mma_k);
          lscLoad<CacheCtrl::L1C_L3C>(mat_a[im][ik], a_block_address);
        }
      }

      // load B
      __ArrayMatrix<T, mma_k, mma_n, DataShuffle::vnni, sg_size>
          mat_b[segment_k][segment_n];
      for (int ik = 0; ik < segment_k; ++ik) {
        AddressPayload<mma_k, mma_n> b_block_address(b_address);
        b_block_address.addSrc0AddrY(ik * mma_k);

        lscLoad<CacheCtrl::L1C_L3C>(mat_b[ik][0], b_block_address);
        for (int in = 1; in < segment_n; ++in) {
          b_block_address.addSrc0AddrX(mma_n);
          lscLoad<CacheCtrl::L1C_L3C>(mat_b[ik][in], b_block_address);
        }
      }

      if constexpr (prefetch_stage != 0) {
        // prefetch a
        for (int im = 0; im < segment_m; ++im) {
          AddressPayload<mma_m, mma_k> prefetch_a_block_address(
              prefetch_a_address);
          prefetch_a_block_address.addSrc0AddrY(im * mma_m);

          lscPrefetch<CacheCtrl::L1C_L3C, T, mma_m, mma_k, DataShuffle::none,
                      sg_size>(prefetch_a_block_address);
          for (int ik = 1; ik < segment_k; ++ik) {
            prefetch_a_block_address.addSrc0AddrX(mma_k);
            lscPrefetch<CacheCtrl::L1C_L3C, T, mma_m, mma_k, DataShuffle::none,
                        sg_size>(prefetch_a_block_address);
          }
        }

        // prefetch b
        for (int ik = 0; ik < segment_k; ++ik) {
          AddressPayload<mma_k, mma_n> prefetch_b_block_address(
              prefetch_b_address);
          prefetch_b_block_address.addSrc0AddrY(ik * mma_k);

          lscPrefetch<CacheCtrl::L1C_L3C, T, mma_k, mma_n, DataShuffle::vnni,
                  sg_size>(prefetch_b_block_address);
          for (int in = 1; in < segment_n; ++in) {
            prefetch_b_block_address.addSrc0AddrX(mma_n);
            lscPrefetch<CacheCtrl::L1C_L3C, T, mma_k, mma_n, DataShuffle::vnni,
                    sg_size>(prefetch_b_block_address);
          }
        }
      }      

      asm("fence_sw");
      // mma
      for (int ik = 0; ik < segment_k; ++ik) {
        for (int im = 0; im < segment_m; ++im) {
          for (int in = 0; in < segment_n; ++in) {
            dpas(acc[im][in], acc[im][in], mat_a[im][ik], mat_b[ik][in]);
          }
        }
      }
      // dpas<mma_m, mma_k, mma_n>(acc, acc, mat_a, mat_b);
      // a_address.addSrc0AddrX(k_stride);
      // b_address.addSrc0AddrY(k_stride);      

      // prefetch_a_address.addSrc0AddrX(k_stride);
      // prefetch_b_address.addSrc0AddrY(k_stride);
    }   
  }

    // store c
    AddressPayload<mma_m, mma_n> c_address(
        (void *)C, (uint32_t)matrix_m,
        static_cast<uint32_t>(matrix_n * sizeof(T)),
        static_cast<uint32_t>(matrix_n * sizeof(T)), n_offset, m_offset);
    for (int im = 0; im < segment_m; ++im) {
      AddressPayload<mma_m, mma_n> c_block_address(c_address);
      c_block_address.addSrc0AddrY(im * mma_m);
      lscStore<CacheCtrl::L1WB_L3WB>(c_block_address, acc[im][0]);

      for (int in = 1; in < segment_n; ++in) {
        c_block_address.addSrc0AddrX(mma_n);
        lscStore<CacheCtrl::L1WB_L3WB>(c_block_address, acc[im][in]);
      }
    }
#else
    auto id = item.get_global_linear_id();
    C[id] = A[id] + B[id];
#endif
  }
  const T *A;
  const T *B;
  T *C;
  const int matrix_m;
  const int matrix_k;
  const int matrix_n;
};

template <typename T>
sycl::event compute_kernel(sycl::queue &q, const T *A, const T *B, T *C,
                           const int matrix_m, const int matrix_k,
                           const int matrix_n) {
  auto num_wg_m = matrix_m / wg_tile_m;
  auto num_wg_n = matrix_n / wg_tile_n;

  sycl::range<3> local_range(wg_size_m, wg_size_n, sg_size);
  sycl::range<3> global_range(num_wg_m, num_wg_n, 1);

  auto evt = q.submit([&](sycl::handler &cgh) {
    MMAKernelImpl<T> task(A, B, C, matrix_m, matrix_k, matrix_n);
    cgh.parallel_for(sycl::nd_range<3>(global_range * local_range, local_range),
                     task);
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

  release_guard __guard([&] {
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
