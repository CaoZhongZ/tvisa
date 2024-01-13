#pragma once

#include <type_traits>
#include <sycl/sycl.hpp>

#define xstr(s) str(s)
#define str(x) #x

enum DataShuffle {
  none, transpose, vnni
};

enum CacheCtrl {
  DEFAULT = 0,
  L1UC_L3UC,
  L1C_L3UC,
  L1C_L3C,
  L1S_L3UC,
};

namespace {

// Enumration space, TODO: compress them with macro string tricks

template <int dataWidth, int vectorSize, int subGroupSize,
         CacheCtrl = CacheCtrl::DEFAULT>
struct LscLoad {
  template <typename T>  static inline void run(T& var, void* addr);
};

template <int dataWidth, int vectorSize, int subGroupSize,
         CacheCtrl = CacheCtrl::DEFAULT>
struct LscStore {
  template <typename T> static inline void run(void* addr, const T& var);
};

// instruction enumeration for all widths and subgroups
//
//
// TODO: Add support for sycl::half and sycl::vec of sycl::half
//
template <> struct LscLoad<1, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d8u32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 4, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 8, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 16, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 4, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 8, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d8u32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 4, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 8, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 16, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 4, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 8, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 16) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 4, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 16) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 1, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d8u32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 2, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 4, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 8, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 16, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 1, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 2, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 4, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 8, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 1, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d8u32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 2, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 4, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 8, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<1, 16, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 1, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 2, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 4, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<2, 8, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 1, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 2, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 4, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<8, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile (
        "lsc_load.ugm (M1, 16) %0:d64 flat[%1]:a64\n" :
        "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<2, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d16c32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<2, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<2, 8, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 1, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 2, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 4, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 4, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 16) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 4, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 16) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
  static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 1, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 32) %0:d32 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 2, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 32) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<4, 4, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 32) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscLoad<8, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile (
        "lsc_load.ugm.uc.uc (M1, 16) %0:d64 flat[%1]:a64\n" :
        "=rw"(var) : "rw" (addr));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<2, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d16c32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<2, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<2, 8, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 1, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 32) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 2, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 32) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 4, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <> struct LscStore<4, 4, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 32) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

//
// XXX: 2-D load can't be used in normal case while can't gaurantee contiguous register
// allocation.
// TODO: figure out a way to do contiguous reigster allocation and represented
// as object.
//
template <int Row, int Wdith, DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct Lsc2DLoad {
  template <typename T> static inline void run(
      T (& array)[Row], void* SurfaceBase,
      int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0Addr);
};

// Register 8x16 load
template <>
struct Lsc2DLoad<8, 16, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    T (& array)[8], void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1_NM, 1) %0:d32.1x8x16nn flat[%1, %2, %3, %4, %5, %6]"
        :: "rw"(array), "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <>
struct Lsc2DLoad<8, 16, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    T (& array)[8], void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load_block2d.ugm.uc.uc (M1_NM, 1) %0:d32.1x8x16nn flat[%1, %2, %3, %4, %5, %6]"
        :: "rw"(array), "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <int Row, int Wdith, DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct Lsc2DStore {
  template <typename T> static inline void run(
      void* SurfaceBase, const T (&array)[Row],
      int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0Addr);
};

template <>
struct Lsc2DStore<8, 16, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    void* SurfaceBase, T (& array)[8],
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store_block2d.ugm (M1_NM, 1) flat[%1, %2, %3, %4, %5, %6] %0:d32.1x8x16nn"
        :: "rw"(array), "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <>
struct Lsc2DStore<8, 16, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    void* SurfaceBase, T (& array)[8],
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_store_block2d.ugm.uc.uc (M1_NM, 1) flat[%1, %2, %3, %4, %5, %6] %0:d32.1x8x16nn"
        :: "rw"(array), "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

template <int Row, int Wdith,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct prefetch2D {
  template <typename T> static inline void run(
      T* SurfaceBase, int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0AddrY);
};

template <> struct prefetch2D<8, 16, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    T* SurfaceBase, int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1_NM, 1) V0:d32.1x8x16nn flat[%1, %2, %3, %4, %5, %6]"
        :: "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
#else
    static_assert(false,
      "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
  }
};

// TODO: enumerateLsc2DLoad(8, 16, DataShuffle::none, CacheCtrl::L1UC_L3UC, "nn", "uc.uc")
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

#define enumerateLsc2DLoad(row, colume, shuffle, cacheCtrl, shuffleStr, cacheStr) \
  template <> struct Lsc2DLoad<row, colume, shuffle, cacheCtrl> {  \
    template <typename T> static inline void run( \
      T (& array)[row], void* SurfaceBase,  \
      int SurfaceWidth, int SurfaceHeight, int SurfacePitch,  \
      int Src0AddrX, int Src0AddrY  \
    ) { \
      asm volatile ("\n"  \
          "lsc_load_block2d.ugm." cacheStr " (M1_NM, 1) %0:d32.1x" str(row) "x" str(colume) shuffleStr " flat[%1, %2, %3, %4, %5, %6]"  \
          :: "rw"(array), "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight), \
          "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));  \
    } \
  };

#else

#define enumerateLsc2DLoad(row, colume, shuffle, cacheCtrl, shuffleStr, cacheStr) \
  template <> struct Lsc2DLoad<row, colume, shuffle, cacheCtrl> {  \
    template <typename T> static inline void run( \
      T (& array)[row], void* SurfaceBase,  \
      int SurfaceWidth, int SurfaceHeight, int SurfacePitch,  \
      int Src0AddrX, int Src0AddrY  \
    ) { \
      static_assert(false,  \
        "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__"); \
    } \
  };

#endif

} // enumrate space

// API
template <int subGroupSize, CacheCtrl CTL= CacheCtrl::DEFAULT, typename T>
static inline void lscLoad(T& var, void *addr) {
  LscLoad<sizeof(T), 1, subGroupSize, CTL>::run(var, addr);
}

template <int subGroupSize, CacheCtrl CTL= CacheCtrl::DEFAULT, typename T>
static inline void lscStore(void *addr, const T& var) {
  LscStore<sizeof(T), 1, subGroupSize, CTL>::run(addr, var);
}

template <int subGroupSize, CacheCtrl CTL= CacheCtrl::DEFAULT, typename T, int N>
static inline void lscLoad(T(& var)[N], void *addr) {
  LscLoad<sizeof(T), N, subGroupSize, CTL>::run(var, addr);
}

template <int subGroupSize, CacheCtrl CTL= CacheCtrl::DEFAULT, typename T, int N>
static inline void lscStore(void *addr, const T(& var)[N]) {
  LscStore<sizeof(T), N, subGroupSize, CTL>::run(addr, var);
}

template <int subGroupSize, CacheCtrl CTL= CacheCtrl::DEFAULT, typename T, int N>
static inline void lscLoad(sycl::vec<T, N>& var, void *addr) {
  LscLoad<sizeof(T), N, subGroupSize, CTL>::run(
      reinterpret_cast<typename sycl::vec<T, N>::vector_t&>(var), addr);
}

template <int subGroupSize, CacheCtrl CTL= CacheCtrl::DEFAULT, typename T, int N>
static inline void lscStore(void* addr, const sycl::vec<T, N>& var) {
  LscStore<sizeof(T), N, subGroupSize, CTL>::run(
      addr, reinterpret_cast<const typename sycl::vec<T, N>::vector_t&>(var));
}

// Intended usage:
//    lscLoad<16, DataShuffle::none, CacheCtrl::L1UC_L3UC>(
//    array, adrs, 1024, 1024, 4096, 0, 0);

template <int Width, DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT,
         typename T, int Row>
static inline void lscLoad(T (&array) [Row], void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
  Lsc2DLoad<Row, Width, Transpose, CTL>::template run<T>(
      array, SurfaceBase,
      SurfaceWidth, SurfaceHeight, SurfacePitch,
      Src0AddrX, Src0AddrY);
}

// Intended usage:
//    lscStore<16, DataShuffle::none, CacheCtrl::L1UC_L3UC>(
//    adrs, array, 1024, 1024, 4096, 0, 0);

template <int Width, DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT,
         typename T, int Row>
static inline void lscStore(void* SurfaceBase, T (&array) [Row],
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
  Lsc2DStore<Row, Width, Transpose, CTL>::template run<T>(
      SurfaceBase, array,
      SurfaceWidth, SurfaceHeight, SurfacePitch,
      Src0AddrX, Src0AddrY);
}
