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
class LscLoad {
  template <typename T>  static inline void run(T& var, void* addr);
};

template <int dataWidth, int vectorSize, int subGroupSize>
class LscStore {
  template <typename T> static inline void run(void* addr, const T& var);
};

// instruction enumeration for all widths and subgroups

template <> class LscLoad<4, 1, 16> {
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

template <> class LscLoad<4, 2, 16> {
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

template <> class LscLoad<4, 4, 16> {
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

template <> class LscLoad<4, 1, 32> {
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

template <> class LscLoad<4, 2, 32> {
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

template <> class LscLoad<4, 4, 32> {
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

template <> class LscStore<4, 1, 16> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%1]:a64, %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> class LscStore<4, 1, 32> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%1]:a64, %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> class LscStore<4, 2, 16> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%1]:a64, %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> class LscStore<4, 2, 32> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%1]:a64, %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> class LscStore<4, 4, 16> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%1]:a64, %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> class LscStore<4, 4, 32> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%1]:a64, %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <int Row, int Wdith, DataShuffle Transpose>
class Lsc2DLoad {
  template <typename T> static inline void run(
      T (&array)[Row], void* SurfaceBase,
      int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0Addr);
};

template <>
class Lsc2DLoad<8, 16, DataShuffle::none> {
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

template <typename T, int subGroupSize,
         typename std::enable_if<std::is_scalar<T>::value, bool>::type = true>
static inline void lscLoad(T& var, void *addr) {
  LscLoad<sizeof(T), 1, subGroupSize>::template run<T>(var, addr);
}

template <typename T, int subGroupSize,
         typename std::enable_if<std::is_scalar<T>::value, bool>::type = true>
static inline void lscStore(void *addr, const T& var) {
  LscStore<sizeof(T), 1, subGroupSize>::template run<T>(addr, var);
}

//
// TODO: seperate compound to array and sycl::vec
//
template <typename T, int subGroupSize,
         typename std::enable_if<std::is_compound<T>::value, bool>::type = true>
static inline void lscLoad(T& var, void *addr) {
  LscLoad<sizeof(T::value_type), T::size(), subGroupSize>::template run<T>(var, addr);
}

template <typename T, int subGroupSize,
         typename std::enable_if<std::is_compound<T>::value, bool>::type = true>
static inline void lscStore(void *addr, const T& var) {
  LscStore<sizeof(T::value_type), T::size(), subGroupSize>::template run<T>(addr, var);
}

// Intended usage:
//    lscLoad<16, DataShuffle::none>(array, adrs, 1024, 1024, 4096, 0, 128);

template <typename T, int Row, int Width, DataShuffle Transpose>
static inline void lscLoad(T (&array) [Row], void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
  Lsc2DLoad<Row, Width, Transpose>::template run<T>(
      array, SurfaceBase,
      SurfaceWidth, SurfaceHeight, SurfacePitch,
      Src0AddrX, Src0AddrY);
}
