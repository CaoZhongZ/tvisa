#pragma once

#include <sycl/sycl.hpp>

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
// TODO: try in real case
void dp4a(uint32_t& dst, uint32_t& accum, uint32_t a, uint32_t b) {
  asm volatile ("dp4a (M1, 16) %0 %1 %2 %3\n":
      "=rw"(dst), "+rw"(accum) : "rw"(a), "rw"(b));
}
#endif

namespace {
struct systolic_config {
  static constexpr int Depth = 8;
  static constexpr int Width = 16;
};

template <
  typename OT,
  typename AccumT,
  typename IT1,
  typename IT2 = IT1,
  typename config = systolic_config
> struct Dpas {
  static constexpr int Depth = config::Depth;
  static constexpr int Width = config::Width;

  static constexpr int OpsPerChan = sizeof(int) / sizeof(IT2);
  static constexpr int Src1ElemsPerChan = sizeof(int) / OpsPerChan;

  static constexpr int K = Depth * OpsPerChan;
  static constexpr int N = Width * Src1ElemsPerChan;

  template <int M> static inline void run(
      __Matrix<OT, M, N, DataShuffle::none>& C,   /* dst */
      __Matrix<AccumT, M, N, DataShuffle::none>& Accum, /* src0 */
      __Matrix<IT1, K, M, DataShuffle::none>& A,  /* src2 */
      __Matrix<IT2, N, K, DataShuffle::vnni>& B   /* src1 */
  );
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

// cover half, PVC
template <typename OT, typename AccumT>
struct Dpas<OT, AccumT, sycl::half, sycl::half, systolic_config> {
  static constexpr int Depth = systolic_config::Depth;
  static constexpr int Width = systolic_config::Width;

  static constexpr int OpsPerChan = sizeof(int) / sizeof(sycl::half); /* 2 */
  static constexpr int Src1ElemsPerChan = 1;

  static constexpr int K = Depth * OpsPerChan; /* 16 */
  static constexpr int N = Width * Src1ElemsPerChan; /* 16 */

  template <int M> static inline void run(
      __Matrix<OT, N, M, DataShuffle::none>&,
      __Matrix<AccumT, N, M, DataShuffle::none>&,
      __Matrix<sycl::half, K, M, DataShuffle::none>&,
      __Matrix<sycl::half, N, K, DataShuffle::vnni>&
  );

// #define GenRepeat(M)  \
//   template <> static inline void run<M>(  \
//       __Matrix<OT, N, M, DataShuffle::none>& C,   /* dst */  \
//       __Matrix<AccumT, N, M, DataShuffle::none>& Accum, /* src0 */ \
//       __Matrix<sycl::half, K, M, DataShuffle::none>& A,  /* src2 */ \
//       __Matrix<sycl::half, N, K, DataShuffle::vnni>& B   /* src1 */ \
//   ) { \
//     asm volatile ("{\n"  \
//         ".decl aliasA v_type=G type=d num_elts=64 align=GRF alias=<%2,0>\n" \
//         ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%3,0>\n"\
//         "dpas.hf.hf.8." str(M) " (M1, 16) %0.0 %1.0 aliasB.0 aliasA(0, 0)\n"  \
//         "}\n" \
//         : "=rw"(C.getStorage()): "rw"(Accum.getStorage()),  \
//         "rw"(A.getStorage()), "rw"(B.getStorage())  \
//     );  \
//   }

#define GenRepeat(M)  \
  template <> static inline void run<M>(  \
      __Matrix<OT, N, M, DataShuffle::none>& C,   /* dst */  \
      __Matrix<AccumT, N, M, DataShuffle::none>& Accum, /* src0 */ \
      __Matrix<sycl::half, K, M, DataShuffle::none>& A,  /* src2 */ \
      __Matrix<sycl::half, N, K, DataShuffle::vnni>& B   /* src1 */ \
  ) { \
    asm volatile ("{\n"  \
        "dpas.hf.hf.8." str(M) " (M1, 16) %0.0 %1.0 %3.0 %2(0, 0)\n"  \
        "}\n" \
        : "=rw"(C.getStorage()): "rw"(Accum.getStorage()),  \
        "rw"(A.getRawStorage()), "rw"(B.getRawStorage())  \
    );  \
  }


  GenRepeat(1);
  GenRepeat(2);
  GenRepeat(3);
  GenRepeat(4);
  // GenRepeat(5);
  // GenRepeat(6);
  GenRepeat(7);
  GenRepeat(8);
#undef GenRepeat
};

// using bf16 = sycl::ext::oneapi::bfloat16;
//
// // cover brain-float, PVC
// template <typename OT, typename AccumT>
// struct Dpas<OT, AccumT, bf16, bf16, systolic_config> {
//   static constexpr int Depth = systolic_config::Depth;
//   static constexpr int Width = systolic_config::Width;
//
//   static constexpr int OpsPerChan = sizeof(int) / sizeof(sycl::half); /* 2 */
//   static constexpr int Src1ElemsPerChan = 1;
//
//   static constexpr int K = Depth * OpsPerChan; /* 16 */
//   static constexpr int N = Width * Src1ElemsPerChan; /* 16 */
//
//   template <int M> static inline void run(
//       __Matrix<OT, M, N, DataShuffle::none>&,
//       __Matrix<AccumT, M, N, DataShuffle::none>&,
//       __Matrix<bf16, K, M, DataShuffle::none>&,
//       __Matrix<bf16, N, K, DataShuffle::vnni>&
//   );
//
// #define GenRepeat(M)  \
//   template <> static inline void run<M>(  \
//       __Matrix<OT, M, N, DataShuffle::none>& C,   /* dst */  \
//       __Matrix<AccumT, M, N, DataShuffle::none>& Accum, /* src0 */ \
//       __Matrix<bf16, K, M, DataShuffle::none>& A,  /* src2 */ \
//       __Matrix<bf16, N, K, DataShuffle::vnni>& B   /* src1 */ \
//   ) { \
//     asm volatile ("\n"  \
//         "dpas.bf.bf.8." str(M) " (M1, 16) %0.0 %1.0 %3.0 %2(0, 0)\n"  \
//         : "=rw"(C.getStorage()): "rw"(Accum.getStorage())  \
//         "rw"(A.getStorage()), "rw"(B.getStorage())  \
//     );  \
//   }
//
//   GenRepeat(1);
//   GenRepeat(2);
//   GenRepeat(3);
//   GenRepeat(4);
//   // GenRepeat(5);
//   // GenRepeat(6);
//   GenRepeat(7);
//   GenRepeat(8);
// #undef GenRepeat
// };


#endif

template <
  int M, int K, int N,
  typename OT,
  typename AccumT,
  typename IT1,
  typename IT2,
  typename config = systolic_config>
static inline void dpas(
    __Matrix<OT, N, M>& D, __Matrix<AccumT, N, M>& C,
    __Matrix<IT1, K, M>& A, __Matrix<IT2, N, K, DataShuffle::vnni>& B
) {
  // TODO: check accepted parameters with static assert;
  Dpas<OT, AccumT, IT1, IT2, systolic_config>::template run<M>(D, C, A, B);
}

}
