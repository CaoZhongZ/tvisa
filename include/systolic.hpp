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

// TODO: DataShuffle of A can be any
  template <int M> static inline void run(
      __ArrayMatrix<OT, M, N, DataShuffle::none>& C,   /* dst */
      __ArrayMatrix<AccumT, M, N, DataShuffle::none>& Accum, /* src0 */
      __ArrayMatrix<IT1, M, K, DataShuffle::none>& A,  /* src2 */
      __ArrayMatrix<IT2, K, N, DataShuffle::vnni>& B   /* src1 */
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
      __ArrayMatrix<OT, M, N, DataShuffle::none>&,
      __ArrayMatrix<AccumT, M, N, DataShuffle::none>&,
      __ArrayMatrix<sycl::half, M, K, DataShuffle::none>&,
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>&
  );

#define GenRepeat(M)  \
  template <> static inline void run<M>(  \
      __ArrayMatrix<OT, M, N, DataShuffle::none>& C,   /* dst */  \
      __ArrayMatrix<AccumT, M, N, DataShuffle::none>& Accum, /* src0 */ \
      __ArrayMatrix<sycl::half, M, K, DataShuffle::none>& A,  /* src2 */ \
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */ \
  ) { \
    asm volatile ("{\n"  \
        ".decl aliasA v_type=G type=d num_elts=64 align=GRF alias=<%2,0>\n" \
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%3,0>\n"\
        "dpas.hf.hf.8." str(M) " (M1, 16) %0.0 %1.0 aliasB.0 aliasA(0, 0)\n"  \
        "}\n" \
        : "=rw"(C.getStorage()): "rw"(Accum.getStorage()),  \
        "rw"(A.getStorage()), "rw"(B.getStorage())  \
    );  \
  }

// Compiler generate shuffle when reinterpret half8 to int4. Wierd, need to confirm
// #define GenRepeat(M)  \
//   template <> static inline void run<M>(  \
//       __ArrayMatrix<OT, M, N, DataShuffle::none>& C,   /* dst */  \
//       __ArrayMatrix<AccumT, M, N, DataShuffle::none>& Accum, /* src0 */ \
//       __ArrayMatrix<sycl::half, M, K, DataShuffle::none>& A,  /* src2 */ \
//       __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */ \
//   ) { \
//     asm volatile ("{\n"  \
//         "dpas.hf.hf.8." str(M) " (M1, 16) %0.0 %1.0 %3.0 %2(0, 0)\n"  \
//         "}\n" \
//         : "=rw"(C.getStorage()): "rw"(Accum.getStorage()),  \
//         "rw"(A.getRawStorage()), "rw"(B.getRawStorage())  \
//     );  \
//   }


  GenRepeat(1);
  GenRepeat(2);
  GenRepeat(3);
  GenRepeat(4);
  // GenRepeat(5);
  // GenRepeat(6);
  GenRepeat(7);
  GenRepeat(8);
#undef GenRepeat

  template <> 
  static inline void run<16>(  
      __ArrayMatrix<OT, 16, N, DataShuffle::none>& C,   /* dst */  
      __ArrayMatrix<AccumT, 16, N, DataShuffle::none>& Accum, /* src0 */ 
      __ArrayMatrix<sycl::half, 16, K, DataShuffle::none>& A,  /* src2 */ 
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */
  ) { 
    asm volatile ("{\n"
        ".decl aliasA0 v_type=G type=d num_elts=128 align=GRF alias=<%2,0>\n"
        ".decl aliasA1 v_type=G type=d num_elts=128 align=GRF alias=<%2,256>\n"
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%3,0>\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %1.0 aliasB.0 aliasA0(0,0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.256 %1.256 aliasB.0 aliasA1(0,0)\n"
        "}\n" 
        : "=rw"(C.getStorage()): "rw"(Accum.getStorage()),  \
        "rw"(A.getStorage()), "rw"(B.getStorage())  \
    );
  }

  template <> 
  static inline void run<32>(  
      __ArrayMatrix<OT, 32, N, DataShuffle::none>& C,   /* dst */  
      __ArrayMatrix<AccumT, 32, N, DataShuffle::none>& Accum, /* src0 */ 
      __ArrayMatrix<sycl::half, 32, K, DataShuffle::none>& A,  /* src2 */ 
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */
  ) { 
    asm volatile ("{\n"
        ".decl aliasA0 v_type=G type=d num_elts=256 align=GRF alias=<%2,0>\n"
        ".decl aliasA1 v_type=G type=d num_elts=256 align=GRF alias=<%2,256>\n"
        ".decl aliasA2 v_type=G type=d num_elts=256 align=GRF alias=<%2,512>\n"
        ".decl aliasA3 v_type=G type=d num_elts=256 align=GRF alias=<%2,768>\n"
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%3,0>\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %1.0 aliasB.0 aliasA0(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.256 %1.256 aliasB.0 aliasA1(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.512 %1.512 aliasB.0 aliasA2(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.768 %1.768 aliasB.0 aliasA3(0, 0)\n"
        "}\n" 
        : "=rw"(C.getStorage()): "rw"(Accum.getStorage()),  \
        "rw"(A.getStorage()), "rw"(B.getStorage())  \
    );  
  }
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
//       __ArrayMatrix<OT, M, N, DataShuffle::none>&,
//       __ArrayMatrix<AccumT, M, N, DataShuffle::none>&,
//       __ArrayMatrix<bf16, M, K, DataShuffle::none>&,
//       __ArrayMatrix<bf16, K, N, DataShuffle::vnni>&
//   );
//
// #define GenRepeat(M)  \
//   template <> static inline void run<M>(  \
//       __ArrayMatrix<OT, M, N, DataShuffle::none>& C,   /* dst */  \
//       __ArrayMatrix<AccumT, M, N, DataShuffle::none>& Accum, /* src0 */ \
//       __ArrayMatrix<bf16, M, K, DataShuffle::none>& A,  /* src2 */ \
//       __ArrayMatrix<bf16, K, N, DataShuffle::vnni>& B   /* src1 */ \
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
  int SubGroupSize,
  typename config = systolic_config>
static inline typename std::enable_if_t<(M <= 32 && K == 16 && N == 16), void> dpas(
    __ArrayMatrix<OT, M, N, DataShuffle::none, SubGroupSize>& D, __ArrayMatrix<AccumT, M, N, DataShuffle::none, SubGroupSize>& C,
    __ArrayMatrix<IT1, M, K,  DataShuffle::none, SubGroupSize>& A, __ArrayMatrix<IT2, K, N, DataShuffle::vnni, SubGroupSize>& B
) {
  // TODO: check accepted parameters with static assert;
  static_assert(SubGroupSize == 16, "SubGroupSize for dpas must be 16");
  Dpas<OT, AccumT, IT1, IT2, systolic_config>::template run<M>(D, C, A, B);
}

template <
  int M, int K, int N,
  typename OT,
  typename AccumT,
  typename IT1,
  typename IT2=IT1,
  int SubGroupSize,
  typename config = systolic_config>
static inline typename std::enable_if_t<std::is_same_v<IT1, sycl::half> && M==32 && K == 16 && N==16, void> dpas(
    __ArrayMatrix<OT, 32, 16, DataShuffle::none, SubGroupSize>(& C)[1][4],
    __ArrayMatrix<AccumT, 32, 16, DataShuffle::none, SubGroupSize>(& Accum)[1][4],
    __ArrayMatrix<IT1, 32, 16,  DataShuffle::none, SubGroupSize>(& A)[1][1], __ArrayMatrix<IT2, 16, 16, DataShuffle::vnni, SubGroupSize>(& B)[1][4]
) {
  // TODO: check accepted parameters with static assert;
  static_assert(SubGroupSize == 16, "SubGroupSize for dpas must be 16");
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("{\n"
        ".decl aliasA0 v_type=G type=d num_elts=256 align=GRF alias=<%8,0>\n"
        ".decl aliasA1 v_type=G type=d num_elts=256 align=GRF alias=<%8,256>\n"
        ".decl aliasA2 v_type=G type=d num_elts=256 align=GRF alias=<%8,512>\n"
        ".decl aliasA3 v_type=G type=d num_elts=256 align=GRF alias=<%8,768>\n"
        ".decl aliasB0 v_type=G type=d num_elts=128 align=GRF alias=<%9,0>\n"
        ".decl aliasB1 v_type=G type=d num_elts=128 align=GRF alias=<%10,0>\n"
        ".decl aliasB2 v_type=G type=d num_elts=128 align=GRF alias=<%11,0>\n"
        ".decl aliasB3 v_type=G type=d num_elts=128 align=GRF alias=<%12,0>\n"
        ".decl aliasC0 v_type=G type=hf num_elts=512 align=GRF alias=<%0,0>\n"
        ".decl aliasC1 v_type=G type=hf num_elts=512 align=GRF alias=<%1,0>\n"
        ".decl aliasC2 v_type=G type=hf num_elts=512 align=GRF alias=<%2,0>\n"
        ".decl aliasC3 v_type=G type=hf num_elts=512 align=GRF alias=<%3,0>\n"
        ".decl aliasAcc0 v_type=G type=hf num_elts=512 align=GRF alias=<%4,0>\n"
        ".decl aliasAcc1 v_type=G type=hf num_elts=512 align=GRF alias=<%5,0>\n"
        ".decl aliasAcc2 v_type=G type=hf num_elts=512 align=GRF alias=<%6,0>\n"
        ".decl aliasAcc3 v_type=G type=hf num_elts=512 align=GRF alias=<%7,0>\n"                
        "dpas.hf.hf.8.8 (M1, 16) aliasC0.0 aliasAcc0.0 aliasB0.0 aliasA0(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC0.256 aliasAcc0.256 aliasB0.0 aliasA1(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC0.512 aliasAcc0.512 aliasB0.0 aliasA2(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC0.768 aliasAcc0.768 aliasB0.0 aliasA3(0, 0)\n"
        
        "dpas.hf.hf.8.8 (M1, 16) aliasC1.0 aliasAcc1.0 aliasB1.0 aliasA0(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC1.256 aliasAcc1.256 aliasB1.0 aliasA1(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC1.512 aliasAcc1.512 aliasB1.0 aliasA2(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC1.768 aliasAcc1.768 aliasB1.0 aliasA3(0, 0)\n"
        
        "dpas.hf.hf.8.8 (M1, 16) aliasC2.0 aliasAcc2.0 aliasB2.0 aliasA0(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC2.256 aliasAcc2.256 aliasB2.0 aliasA1(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC2.512 aliasAcc2.512 aliasB2.0 aliasA2(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC2.768 aliasAcc2.768 aliasB2.0 aliasA3(0, 0)\n"
        
        "dpas.hf.hf.8.8 (M1, 16) aliasC3.0 aliasAcc3.0 aliasB3.0 aliasA0(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC3.256 aliasAcc3.256 aliasB3.0 aliasA1(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC3.512 aliasAcc3.512 aliasB3.0 aliasA2(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC3.768 aliasAcc3.768 aliasB3.0 aliasA3(0, 0)\n"                     
        "}\n" 
        : "=rw"(C[0][0].getStorage()), "=rw"(C[0][1].getStorage()), "=rw"(C[0][2].getStorage()), "=rw"(C[0][3].getStorage()) \
        : "rw"(Accum[0][0].getStorage()), "rw"(Accum[0][1].getStorage()), "rw"(Accum[0][2].getStorage()), "rw"(Accum[0][3].getStorage()),
        "rw"(A[0][0].getStorage()), \
        "rw"(B[0][0].getStorage()), "rw"(B[0][1].getStorage()), "rw"(B[0][2].getStorage()), "rw"(B[0][3].getStorage())
    );  
#endif  
}

}
