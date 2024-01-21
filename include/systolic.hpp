#pragma once

#include <sycl/sycl.hpp>

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
// TODO: try in real case
void dp4a(uint32_t& dst, uint32_t& accum, uint32_t a, uint32_t b) {
  asm volatile ("dp4a (M1, 16) %0 %1 %2 %3\n":
      "=rw"(dst), "+rw"(accum) : "rw"(a), "rw"(b));
}
#endif

// TODO: define matrix shape out on sycl::vec<T, N>::vector_t
enum DataType {
  HF,
};

namespace {
//
// let's try strong type, reordered shape for low-level type
//

template <typename OutputType, typename AccumType, typename InputType>
struct Dpas {
  template <int Row_C, int Row_A, int Row_B, int STEP> static inline void run(
      sycl::vec<OutputType, Raw_A>& C,
      sycl::vec<AccumType, Raw_A>& Accum,
      sycl::vec<InputType, Row_A>& A,
      sycl::vec<InputType, Row_B>& B
  );
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

template <> struct Dpas<float, float, sycl::half> {
  // C = ACC + A * B
  template <> static inline void run<8, 4, 16, 8>(
      sycl::vec<float, 8>::vector_t& C,   /* 8 x 16 */
      sycl::vec<float, 8>::vector_t& ACC,  /* 8 x 16 */
      sycl::vec<sycl::half, 4>::vector_t& A, /* 8 x 16 */
      sycl::vec<sycl::half, 16>::vector_t& B, /* 16 x 16 */
  ) {
    asm volatile ("\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %1.0 %3.0 %2(0, 0)\n"
        : "=rw"(C), "+rw"(ACC) : "rw"(A), "rw"(B)
    );
  }
};

template <> struct Dpas<sycl::half, float, sycl::half> {
  // C = ACC + A * B
  template <> static inline void run<8, 4, 16, 8>(
      sycl::vec<sycl::half, 8>::vector_t& C,   /* 8 x 16, layout??? */
      sycl::vec<sycl::half, 8>::vector_t& ACC,  /* 8 x 16 */
      sycl::vec<sycl::half, 4>::vector_t& A, /* 8 x 16 */
      sycl::vec<sycl::half, 16>::vector_t& B, /* 16 x 16 */
  ) {
    asm volatile ("\n"
        "dpas.hf.hf.8.8 (M1, 16) %0.0 %1.0 %3.0 %2(0, 0)\n"
        : "=rw"(C), "+rw"(ACC) : "rw"(A), "rw"(B)
    );
  }
};

#endif

}
