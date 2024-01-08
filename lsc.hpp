#pragma once

#include <type_traits>
#include <sycl/sycl.hpp>

enum DataShuffle {
  none, transpose, vnni
};

enum CacheCtrl {
  L1STATE_L3MOCS = 0,
  L1UC_L3UC,
  L1C_L3UC,
  L1C_L3C,
  L1S_L3UC,
};

namespace {

template <int dataWidth, int vectorSize, int subGroupSize>
struct LscLoad {
  template <typename T>  static inline void run(T& var, void* addr);
};

template <int dataWidth, int vectorSize, int subGroupSize>
struct LscStore {
  template <typename T> static inline void run(void* addr, const T& var);
};

// instruction enumeration for all widths and subgroups

template <> struct LscLoad<4, 1, 16> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
    var = *reinterpret_cast<T *>(addr);
#endif
  }
};

template <> struct LscLoad<4, 2, 16> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 16) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 4, 16> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 16) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 1, 32> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 2, 32> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 4, 32> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<8, 1, 16> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile (
        "lsc_load.ugm (M1, 16) %0:d64 flat[%1]:a64\n" :
        "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 1, 16> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%1]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 1, 32> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%1]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 2, 16> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 2, 32> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 4, 16> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 4, 32> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <int Row, int Wdith, DataShuffle Transpose>
struct Lsc2DLoad {
  template <typename T> static inline void run(
      T (&array)[Row], void* SurfaceBase,
      int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0Addr);
};

template <>
struct Lsc2DLoad<8, 16, DataShuffle::none> {
  template <typename T> static inline void run(
    T (& array)[8], void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1, 1) %0:d32.1x8x16nn flat[%1, %2, %3, %4, %5, %6]"
        :: "rw"(array), "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

} // enumrate space

template <int subGroupSize, typename T>
static inline void lscLoad(T& var, void *addr) {
  LscLoad<sizeof(T), 1, subGroupSize>::run(var, addr);
}

template <int subGroupSize, typename T>
static inline void lscStore(void *addr, const T& var) {
  LscStore<sizeof(T), 1, subGroupSize>::run(addr, var);
}

template <int subGroupSize, typename T, int N>
static inline void lscLoad(T(& var)[N], void *addr) {
  LscLoad<sizeof(T), N, subGroupSize>::run(var, addr);
}

template <int subGroupSize, typename T, int N>
static inline void lscStore(void *addr, const T(& var)[N]) {
  LscStore<sizeof(T), N, subGroupSize>::run(addr, var);
}

template <int subGroupSize, typename T, int N>
static inline void lscLoad(sycl::vec<T, N>& var, void *addr) {
  LscLoad<sizeof(T), N, subGroupSize>::run(var, addr);
}

template <int subGroupSize, typename T, int N>
static inline void lscStore(void* addr, const sycl::vec<T, N>& var) {
  LscStore<sizeof(T), N, subGroupSize>::run(addr, var);
}

// Intended usage:
//    lscLoad<16, DataShuffle::none>(array, adrs, 1024, 1024, 4096, 0, 128);

template <int Width, DataShuffle Transpose, typename T, int Row>
static inline void lscLoad(T (&array) [Row], void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
  Lsc2DLoad<Row, Width, Transpose>::template run<T>(
      array, SurfaceBase,
      SurfaceWidth, SurfaceHeight, SurfacePitch,
      Src0AddrX, Src0AddrY);
}
