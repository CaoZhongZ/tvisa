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
  typename IT2 = IT1
> struct Dpas32x16x32 {
  static constexpr int M = 32;
  static constexpr int K = 16;
  static constexpr int N = 32;
  static constexpr int SubGroupSize = 16;
  static inline void run(
      __ArrayMatrix<OT, M, N/2, DataShuffle::none, SubGroupSize>(& C)[2],   /* dst/src0 */
      __ArrayMatrix<IT1, M, K, DataShuffle::none, SubGroupSize>& A,  /* src2 */
      __ArrayMatrix<IT2, K, N/2, DataShuffle::vnni, SubGroupSize, 2>& B   /* src1 */
  );
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
template <>
struct Dpas32x16x32<sycl::half, sycl::half, sycl::half, sycl::half> {
  static constexpr int M = 32;
  static constexpr int K = 16;
  static constexpr int N = 32;  
  static constexpr int SubGroupSize = 16;
  
   static inline void run(
      __ArrayMatrix<sycl::half, M, N/2, DataShuffle::none, SubGroupSize>(& C)[2],  /* dst/src0 */
      __ArrayMatrix<sycl::half, M, K, DataShuffle::none, SubGroupSize>& A,  /* src2 */
      __ArrayMatrix<sycl::half, K, N/2, DataShuffle::vnni, SubGroupSize, 2>& B   /* src1 */
  ) {
    asm volatile ("{\n"  
        ".decl aliasC0 v_type=G type=hf num_elts=512 align=GRF alias=<%0,0>\n"     
        ".decl aliasC1 v_type=G type=hf num_elts=512 align=GRF alias=<%1,0>\n"     
        ".decl aliasA v_type=G type=d num_elts=256 align=GRF alias=<%2,0>\n" 
        ".decl aliasB0 v_type=G type=d num_elts=128 align=GRF alias=<%3,0>\n"
        ".decl aliasB1 v_type=G type=d num_elts=128 align=GRF alias=<%3,512>\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC0.0 aliasC0.0 aliasB0.0 aliasA(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC0.256 aliasC0.256 aliasB0.0 aliasA(4, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC0.512 aliasC0.512 aliasB0.0 aliasA(8, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC0.768 aliasC0.768 aliasB0.0 aliasA(12, 0)\n" 
        "dpas.hf.hf.8.8 (M1, 16) aliasC1.0 aliasC1.0 aliasB1.0 aliasA(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC1.256 aliasC1.256 aliasB1.0 aliasA(4, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC1.512 aliasC1.512 aliasB1.0 aliasA(8, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) aliasC1.768 aliasC1.768 aliasB1.0 aliasA(12, 0)\n"                  
        "}\n" 
        : "+rw"(C[0].getStorage()), "+rw"(C[1].getStorage()) \
        : "rw"(A.getStorage()), "rw"(B.getStorage())
    );      
  };
};
#endif

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

  template <int M> static inline void run(
      __ArrayMatrix<OT, M, N, DataShuffle::none>& C,   /* dst/src0 */
      __ArrayMatrix<IT1, M, K, DataShuffle::none>& A,  /* src2 */
      __ArrayMatrix<IT2, K, N, DataShuffle::vnni>& B   /* src1 */
  );
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

// cover half, PVC
template <>
struct Dpas<sycl::half, sycl::half, sycl::half, sycl::half, systolic_config> {
  static constexpr int Depth = systolic_config::Depth;
  static constexpr int Width = systolic_config::Width;

  static constexpr int OpsPerChan = sizeof(int) / sizeof(sycl::half); /* 2 */
  static constexpr int Src1ElemsPerChan = 1;

  static constexpr int K = Depth * OpsPerChan; /* 16 */
  static constexpr int N = Width * Src1ElemsPerChan; /* 16 */

  template <int M> static inline void run(
      __ArrayMatrix<sycl::half, M, N, DataShuffle::none>&,
      __ArrayMatrix<sycl::half, M, N, DataShuffle::none>&,
      __ArrayMatrix<sycl::half, M, K, DataShuffle::none>&,
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>&
  );

  template <int M> static inline void run(
      __ArrayMatrix<sycl::half, M, N, DataShuffle::none>&,
      __ArrayMatrix<sycl::half, M, K, DataShuffle::none>&,
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>&
  );

#define GenRepeat(M)  \
  template <> inline void run<M>(  \
      __ArrayMatrix<sycl::half, M, N, DataShuffle::none>& C,   /* dst */  \
      __ArrayMatrix<sycl::half, M, N, DataShuffle::none>& Accum, /* src0 */ \
      __ArrayMatrix<sycl::half, M, K, DataShuffle::none>& A,  /* src2 */ \
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */ \
  ) { \
    asm volatile ("{\n"  \
        ".decl aliasA v_type=G type=d num_elts=64 align=GRF alias=<%2,0>\n" \
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%3,0>\n"\
        "dpas.hf.hf.8." str(M) " (M1, 16) %0.0 %1.0 aliasB.0 aliasA(0, 0)\n"  \
        "}\n" \
        : "+rw"(C.getStorage()): "rw"(Accum.getStorage()),  \
        "rw"(A.getStorage()), "rw"(B.getStorage())  \
    );  \
  } \
  \
  template <> inline void run<M>(  \
      __ArrayMatrix<sycl::half, M, N, DataShuffle::none>& C,   /* dst */  \
      __ArrayMatrix<sycl::half, M, K, DataShuffle::none>& A,  /* src2 */ \
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */ \
  ) { \
    asm volatile ("{\n"  \
        ".decl aliasA v_type=G type=d num_elts=64 align=GRF alias=<%1,0>\n" \
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%2,0>\n"\
        "dpas.hf.hf.8." str(M) " (M1, 16) %0.0 %0.0 aliasB.0 aliasA(0, 0)\n"  \
        "}\n" \
        : "+rw"(C.getStorage()): "rw"(A.getStorage()), "rw"(B.getStorage())  \
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
//         : "+rw"(C.getStorage()): "rw"(Accum.getStorage()),  \
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
  inline void run<16>(
      __ArrayMatrix<sycl::half, 16, N, DataShuffle::none>& C,   /* dst */
      __ArrayMatrix<sycl::half, 16, N, DataShuffle::none>& Accum, /* src0 */
      __ArrayMatrix<sycl::half, 16, K, DataShuffle::none>& A,  /* src2 */
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */
  ) {
    asm volatile ("{\n"
        ".decl aliasA v_type=G type=d num_elts=128 align=GRF alias=<%2,0>\n"
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%3,0>\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %1.0 aliasB.0 aliasA(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.256 %1.256 aliasB.0 aliasA(4, 0)\n"
        "}\n"
        : "+rw"(C.getStorage()): "rw"(Accum.getStorage()),
        "rw"(A.getStorage()), "rw"(B.getStorage())
    );
  }

  template <>
  inline void run<16>(
      __ArrayMatrix<sycl::half, 16, N, DataShuffle::none>& C,   /* dst */
      __ArrayMatrix<sycl::half, 16, K, DataShuffle::none>& A,  /* src2 */
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */
  ) {
    asm volatile ("{\n"
        ".decl aliasA v_type=G type=d num_elts=128 align=GRF alias=<%1,0>\n"
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%2,0>\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %0.0 aliasB.0 aliasA(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.256 %0.256 aliasB.0 aliasA(4, 0)\n"
        "}\n"
        : "+rw"(C.getStorage()): "rw"(A.getStorage()), "rw"(B.getStorage())
    );
  }

  template <>
  inline void run<32>(
      __ArrayMatrix<sycl::half, 32, N, DataShuffle::none>& C,   /* dst */
      __ArrayMatrix<sycl::half, 32, N, DataShuffle::none>& Accum, /* src0 */
      __ArrayMatrix<sycl::half, 32, K, DataShuffle::none>& A,  /* src2 */
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */
  ) {
    asm volatile ("{\n"
        ".decl aliasA v_type=G type=d num_elts=256 align=GRF alias=<%2,0>\n"
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%3,0>\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %1.0 aliasB.0 aliasA(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.256 %1.256 aliasB.0 aliasA(4, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.512 %1.512 aliasB.0 aliasA(8, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.768 %1.768 aliasB.0 aliasA(12, 0)\n"
        "}\n"
        : "+rw"(C.getStorage()): "rw"(Accum.getStorage()),
        "rw"(A.getStorage()), "rw"(B.getStorage())
    );
  }

#define repeatDpas32(dstType, srcType, dstTLen, srcTLen) \
  template <> inline void run<32>(  \
      __ArrayMatrix<dstType, 32, N, DataShuffle::none>& C,   /* dst */  \
      __ArrayMatrix<srcType, 32, K, DataShuffle::none>& A,  /* src2 */ \
      __ArrayMatrix<srcType, K, N, DataShuffle::vnni>& B   /* src1 */\
  ) { \
    asm volatile ("{\n"\
        ".decl aliasA v_type=G type=d num_elts=" xstr(32 * 16 * srcTLen / 4) " align=GRF alias=<%1,0>\n"\
        ".decl aliasB v_type=G type=d num_elts=" xstr(16 * 16 * srcTLen / 4) " align=GRF alias=<%2,0>\n"\
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %0.0 aliasB.0 aliasA(0, 0)\n" \
        "dpas.hf.hf.8.8 (M1, 16) %0." xstr(1 * 8 * 16 * dstTLen) " %0." xstr(1 * 8 * 16 * dstTLen) " aliasB.0 aliasA(" xstr(srcTLen * 2) ", 0)\n" \
        "dpas.hf.hf.8.8 (M1, 16) %0." xstr(2 * 8 * 16 * dstTLen) " %0." xstr(2 * 8 * 16 * dstTLen) " aliasB.0 aliasA(" xstr(srcTLen * 4) ", 0)\n" \
        "dpas.hf.hf.8.8 (M1, 16) %0." xstr(3 * 8 * 16 * dstTLen) " %0." xstr(3 * 8 * 16 * dstTLen) " aliasB.0 aliasA(" xstr(srcTLen * 6) ", 0)\n"  \
        "}\n"   \
        : "+rw"(C.getStorage()): "rw"(A.getStorage()), "rw"(B.getStorage()) \
    );  \
  }

  // XXX: test it
  // repeatDpas32(float, sycl::half, 4, 2);
  // repeatDpas32(sycl::half, sycl::half, 2, 2);

  template <> inline void run<32>(
      __ArrayMatrix<sycl::half, 32, N, DataShuffle::none>& C,   /* dst */
      __ArrayMatrix<sycl::half, 32, K, DataShuffle::none>& A,  /* src2 */
      __ArrayMatrix<sycl::half, K, N, DataShuffle::vnni>& B   /* src1 */
  ) {
    asm volatile ("{\n"
        ".decl aliasA v_type=G type=d num_elts=256 align=GRF alias=<%1,0>\n"
        ".decl aliasB v_type=G type=d num_elts=128 align=GRF alias=<%2,0>\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %0.0 aliasB.0 aliasA(0, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.256 %0.256 aliasB.0 aliasA(4, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.512 %0.512 aliasB.0 aliasA(8, 0)\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.768 %0.768 aliasB.0 aliasA(12, 0)\n"
        "}\n"
        : "+rw"(C.getStorage()): "rw"(A.getStorage()), "rw"(B.getStorage())
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
static inline void dpas(
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
  typename IT1,
  typename IT2,
  int SubGroupSize,
  typename config = systolic_config>
static typename std::enable_if<K == 16 && N == 16, void>::type dpas(
    __ArrayMatrix<OT, M, N, DataShuffle::none, SubGroupSize>& C,
    __ArrayMatrix<IT1, M, K,  DataShuffle::none, SubGroupSize>& A,
    __ArrayMatrix<IT2, K, N, DataShuffle::vnni, SubGroupSize>& B
) {
  // TODO: check accepted parameters with static assert;
  static_assert(SubGroupSize == 16, "SubGroupSize for dpas must be 16");
  Dpas<OT, OT, IT1, IT2, systolic_config>::template run<M>(C, A, B);
}

template <
  int M, int K, int N,
  typename OT,
  typename IT1,
  typename IT2,
  int SubGroupSize>
static inline typename std::enable_if<M == 32 && K == 16 && N == 32, void>::type dpas(
    __ArrayMatrix<OT, M, N/2, DataShuffle::none, SubGroupSize>(& C)[2],
    __ArrayMatrix<IT1, M, K, DataShuffle::none, SubGroupSize>& A,
    __ArrayMatrix<IT2, K, N / 2, DataShuffle::vnni, SubGroupSize, 2>& B
) {
  // TODO: check accepted parameters with static assert;
  static_assert(SubGroupSize == 16, "SubGroupSize for dpas must be 16");
  Dpas32x16x32<OT, OT, IT1, IT2>::run(C, A, B);
}

}
