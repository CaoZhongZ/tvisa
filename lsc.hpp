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

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
// instruction enumeration for all widths and subgroups
//
//
// TODO: Add support for sycl::half and sycl::vec of sycl::half
//
template <> struct LscLoad<1, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d8u32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 4, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 8, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 16, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 4, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 8, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d8u32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 4, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 8, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 16, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 4, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 8, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<4, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<4, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm (M1, 16) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<4, 4, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm (M1, 16) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<1, 1, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d8u32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 2, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 4, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 8, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 16, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 1, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 2, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 4, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 8, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm (M1, 32) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 1, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d8u32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 2, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 4, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 8, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<1, 16, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 1, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d16c32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 2, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 4, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32x2 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<2, 8, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 32) %0:d32x4 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<4, 1, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<4, 2, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<4, 4, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm (M1, 32) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<8, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile (
        "lsc_load.ugm (M1, 16) %0:d64 flat[%1]:a64\n" :
        "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscStore<2, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d16c32\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<2, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<2, 8, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 1, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 1, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 2, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 2, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 4, 16, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 4, 32, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm (M1, 32) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscLoad<4, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
      "lsc_load.ugm.uc.uc (M1, 16) %0:d32 flat[%1]:a64\n"
      : "=rw"(var) : "rw"(addr));
  }
};

template <> struct LscLoad<4, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 16) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<4, 4, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 16) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<4, 1, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 32) %0:d32 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<4, 2, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 32) %0:d32x2 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<4, 4, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile ("\n"
        "lsc_load.ugm.uc.uc (M1, 32) %0:d32x4 flat[%1]:a64\n"
        : "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscLoad<8, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(T& var, void* addr) {
    asm volatile (
        "lsc_load.ugm.uc.uc (M1, 16) %0:d64 flat[%1]:a64\n" :
        "=rw"(var) : "rw" (addr));
  }
};

template <> struct LscStore<2, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d16c32\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<2, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<2, 8, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 1, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 1, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 32) flat[%0]:a64 %1:d32\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 2, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 2, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 32) flat[%0]:a64 %1:d32x2\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 4, 16, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 16) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
  }
};

template <> struct LscStore<4, 4, 32, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(void *addr, const T& var) {
    asm volatile ("\n"
        "lsc_store.ugm.uc.uc (M1, 32) flat[%0]:a64 %1:d32x4\n"
        :: "rw"(addr), "rw"(var));
  }
};
#endif
//
// Register allocation
//  Only OpenCL vector type could gurantee IGC do contiguous allocation
//
template <int BlockWdith, int BlockHeight,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct Lsc2DLoad {
  template <typename T> static inline void run(
      sycl::vec<T, BlockHeight>& array, void* SurfaceBase,
      int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0Addr);
};


#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
template <>
struct Lsc2DLoad<16, 8, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1_NM, 1) %0:d32.1x16x8nn flat[%1, %2, %3, %4, %5, %6]"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t&>(array)) :
        "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
  }
};

template <>
struct Lsc2DLoad<16, 8, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
    asm volatile ("\n"
        "lsc_load_block2d.ugm.uc.uc (M1_NM, 1) %0:d32.1x16x8nn flat[%1, %2, %3, %4, %5, %6]"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t&>(array)) :
        "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
  }
};

template <>
struct Lsc2DLoad<16, 4, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    sycl::vec<T, 4>& array, void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1_NM, 1) %0:d32.1x16x4nn flat[%1, %2, %3, %4, %5, %6]"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 4>::vector_t&>(array)) :
        "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
  }
};

template <>
struct Lsc2DLoad<16, 4, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    sycl::vec<T, 4>& array, void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
    asm volatile ("\n"
        "lsc_load_block2d.ugm.uc.uc (M1_NM, 1) %0:d32.1x16x4nn flat[%1, %2, %3, %4, %5, %6]"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 4>::vector_t&>(array)) :
        "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
  }
};
#endif

template <int BlockWdith, int BlockHeight,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct Lsc2DStore {
  template <typename T> static inline void run(
      void* SurfaceBase, const sycl::vec<T, BlockHeight>& array,
      int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0Addr);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
template <>
struct Lsc2DStore<16, 8, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    void* SurfaceBase, sycl::vec<T, 8>& array,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
    asm volatile ("\n"
        "lsc_store_block2d.ugm (M1_NM, 1) flat[%1, %2, %3, %4, %5, %6] %0:d32.1x16x8nn"
        :: "rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t &>(array)),
        "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
  }
};

template <>
struct Lsc2DStore<16, 8, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    void* SurfaceBase, sycl::vec<T, 8>& array,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY
  ) {
    asm volatile ("\n"
        "lsc_store_block2d.ugm.uc.uc (M1_NM, 1) flat[%1, %2, %3, %4, %5, %6] %0:d32.1x16x8nn"
        :: "rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t &>(array)),
        "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
  }
};
#endif

template <int BlockWidth, int BlockHeight,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct prefetch2D {
  template <typename T> static inline void run(
      T* SurfaceBase, int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0AddrY);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
template <> struct prefetch2D<16, 8, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    T* SurfaceBase, int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1_NM, 1) V0:d32.1x16x8nn flat[%1, %2, %3, %4, %5, %6]"
        :: "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
  }
};

// TODO: enumerateLsc2DLoad(8, 16, DataShuffle::none, CacheCtrl::L1UC_L3UC, "nn", "uc.uc")

#define enumerateLsc2DLoad(width, height, shuffle, cacheCtrl, shuffleStr, cacheStr) \
  template <> struct Lsc2DLoad<width, height, shuffle, cacheCtrl> {  \
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

#endif
} // enumrate space

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

template <int BlockWidth, int BlockHeight, int ArrayLength = 1>
struct AddressPayload {
  inline AddressPayload(const AddressPayload& payload) {
    payloadReg_ = payload.getPayload();
  }

  inline AddressPayload(
    void* SurfaceBase,
    uint32_t SurfaceWidth, uint32_t SurfaceHeight,
    uint32_t SurfacePitch, int Src0AddrX, int Src0AddrY
  ) {
    constexpr uint32_t BWHAL = (BlockWidth -1)
      | (BlockHeight -1) << 8 | ((ArrayLength -1) & 0xf) << 16;

    asm volatile ("{\n"
        ".decl alias64 v_type=G type=uq num_elts=8 align=GRF alias=<%0, 0>\n"
        "mov (M1, 1) alias64(0, 0)<1> %1(0, 0)<0;1,0>\n"
        "add (M1, 1) %0(0, 2)<1> %2(0, 0)<0;1,0> -1:ud\n"
        "add (M1, 1) %0(0, 3)<1> %3(0, 0)<0;1,0> -1:ud\n"
        "add (M1, 1) %0(0, 4)<1> %4(0, 0)<0;1,0> -1:ud\n"
        "mov (M1, 1) %0(0, 5)<1> %5(0, 0)<0;1,0> \n"
        "mov (M1, 1) %0(0, 6)<1> %6(0, 0)<0;1,0> \n"
        "mov (M1, 1) %0(0, 7)<1> %7\n"
        "}\n"
        : "+rw"(payloadReg_) : "rw"(SurfaceBase), "rw"(SurfaceWidth),
        "rw"(SurfaceHeight), "rw"(SurfacePitch), "rw"(Src0AddrX),
        "rw"(Src0AddrY), "i"(BWHAL)
        /*
        : "+rw"(payloadReg_) : "rw.u"(SurfaceBase), "rw.u"(SurfaceWidth),
        "rw.u"(SurfaceHeight), "rw.u"(SurfacePitch), "rw.u"(Src0AddrX),
        "rw.u"(Src0AddrY), "i"(BWHAL)
        */
    );
  }

  inline AddressPayload& operator = (const AddressPayload& payload) {
    payloadReg_ = payload.getPayload();
    return payloadReg_;
  }

  inline uint32_t& getPayload() {
    return payloadReg_;
  }

  inline uint32_t& addSrc0AddrX(int offset) {
    asm volatile (
        "add (M1, 1) %0(0, 5)<1> %0(0, 5)<0;1,0> %1(0,0)<0;1,0>\n"
        : "+rw"(payloadReg_) : "rw"(offset)
    );
    return payloadReg_;
  }

  inline uint32_t& addSrc0AddrY(int offset) {
    asm volatile (
        "add (M1, 1) %0(0, 6)<1> %0(0, 6)<0;1,0> %1(0,0)<0;1,0>\n"
        : "+rw"(payloadReg_) : "rw"(offset)
    );
    return payloadReg_;
  }

  inline uint32_t& updateSurfaceBase(void *addr) {
    asm volatile ("{\n"
        ".decl alias64 v_type=G type=uq num_elts=8 align=GRF alias=<%0, 0>\n"
        "mov (M1, 1) alias64(0, 0)<1> %1(0, 0)<0;1,0>\n"
        "}\n"
        : "+rw"(payloadReg_) : "rw"(addr)
    );
    return payloadReg_;
  }

  inline uint32_t& updateSrc0AddrX(int x_off) {
    asm volatile ("{\n"
        "mov (M1, 1) %0(0, 5)<1> %1(0, 0)<0;1,0>\n"
        "}\n"
        : "=rw"(payloadReg_) : "rw"(x_off)
    );
    return payloadReg_;
  }

  inline uint32_t& updateSrc0AddrY(int y_off) {
    asm volatile ("{\n"
        "mov (M1, 1) %0(0, 6)<1> %1(0, 0)<0;1,0>\n"
        "}\n"
        : "=rw"(payloadReg_) : "rw"(y_off)
    );
    return payloadReg_;
  }

  // fork base and return new one
  inline AddressPayload cloneUpdateSurfaceBase(void * addr) const {
    return updateSurfaceBase(addr);
  }
private:
  uint32_t payloadReg_;
};

template <int BlockWidth, int BlockHeight, int ArrayLength>
static inline uint32_t packAddressPayload (
    void* SurfaceBase,
    uint32_t SurfaceWidth, uint32_t SurfaceHeight, uint32_t SurfacePitch,
    int Src0AddrX, int Src0AddrY
) {
  uint32_t addressPayload;
  constexpr uint32_t BWHAL = (BlockWidth-1)
    | (BlockHeight-1) << 8 | ((ArrayLength -1) & 0xf) << 16;

  asm volatile ("{\n"
      ".decl alias64 v_type=G type=uq num_elts=8 align=GRF alias=<%0, 0>\n"
      "mov (M1, 1) alias64(0, 0)<1> %1(0, 0)<0;1,0>\n"
      "add (M1, 1) %0(0, 2)<1> %2(0, 0)<0;1,0> -1:ud\n"
      "add (M1, 1) %0(0, 3)<1> %3(0, 0)<0;1,0> -1:ud\n"
      "add (M1, 1) %0(0, 4)<1> %4(0, 0)<0;1,0> -1:ud\n"
      "mov (M1, 1) %0(0, 5)<1> %5(0, 0)<0;1,0> \n"
      "mov (M1, 1) %0(0, 6)<1> %6(0, 0)<0;1,0> \n"
      "mov (M1, 1) %0(0, 7)<1> %7\n"
      "}\n"
      : "+rw"(addressPayload) : "rw"(SurfaceBase), "rw"(SurfaceWidth),
      "rw"(SurfaceHeight), "rw"(SurfacePitch), "rw"(Src0AddrX),
      "rw"(Src0AddrY), "i"(BWHAL)
      /*
      : "+rw"(addressPayload) : "rw.u"(SurfaceBase), "rw.u"(SurfaceWidth),
      "rw.u"(SurfaceHeight), "rw.u"(SurfacePitch), "rw.u"(Src0AddrX),
      "rw.u"(Src0AddrY), "i"(BWHAL)
      */
  );

  return addressPayload;
}

static inline uint32_t& updateBaseAddress(uint32_t &addressPayload, void* base) {
  asm volatile ("{\n"
      ".decl alias64 v_type=G type=uq num_elts=8 align=GRF alias=<%0, 0>\n"
      "mov (M1, 1) alias64(0, 0)<1> %1(0, 0)<0;1,0>\n"
      "}\n"
      : "+rw"(addressPayload) : "rw"(base)
  );
  return addressPayload;
}

#endif

// more load/store instruction enumeration
namespace {

template <int BlockWdith, int BlockHeight,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct Lsc2DLoad {
  template <typename T> static inline void run(
      sycl::vec<T, BlockHeight>& array,
      AddressPayload<BlockWidth, BlockHeight>& address);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
//[(<P>)] {raw_sends|raw_sendsc}[_eot] <sfid> <numSrc0> <numSrc1> <numDst> (<exec_size>) <ExMsgDesc> <Desc> <Src0> <Src1> <Dst>

template <>
struct Lsc2DLoad<16, 8, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, AddressPayload<16, 8>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.8 (M1, 1) 0x0:ud 0x2800403:ud %1 V0 %0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};

template <>
struct Lsc2DLoad<16, 8, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, AddressPayload<16, 8>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.8 (M1, 1) 0x0:ud 0x2820403:ud %1 V0 %0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};

template <>
struct Lsc2DLoad<16, 4, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, AddressPayload<16, 4>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.4 (M1, 1) 0x0:ud 0x2400403:ud %1 V0 %0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 4>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};

template <>
struct Lsc2DLoad<16, 4, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, AddressPayload<16, 4>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.4 (M1, 1) 0x0:ud 0x2420403:ud %1 V0 %0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 4>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};
#endif

template <int BlockWdith, int BlockHeight,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct Lsc2DStore {
  template <typename T> static inline void run(
      AddressPayload<BlockWidth, BlockHeight>& address,
      const sycl::vec<T, BlockHeight>& array);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

template <>
struct Lsc2DStore<16, 8, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
      AddressPayload<16, 8>& address, const sycl::vec<T, 8>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.8.0 (M1, 1) 0x0:ud 0x2800407:ud %0 %1 V0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t&>(array))
    );
  }
};

template <>
struct Lsc2DStore<16, 8, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
      AddressPayload<16, 8>& address, const sycl::vec<T, 8>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.8.0 (M1, 1) 0x0:ud 0x2820407:ud %0 %1 V0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t&>(array))
  }
};

template <>
struct Lsc2DStore<16, 4, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
      AddressPayload<16, 4>& address, const sycl::vec<T, 4>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.8.0 (M1, 1) 0x0:ud 0x2400407:ud %0 %1 V0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<typename sycl::vec<T, 4>::vector_t&>(array))
    );
  }
};

template <>
struct Lsc2DStore<16, 4, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
      AddressPayload<16, 4>& address, const sycl::vec<T, 4>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.8.0 (M1, 1) 0x0:ud 0x2420407:ud %0 %1 V0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<typename sycl::vec<T, 4>::vector_t&>(array))
  }
};

#endif

}

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

template <int BlockWidth, DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT,
         typename T, int BlockHeight>
static inline void lscLoad(sycl::vec<T, BlockHeight>& array, void* SurfaceBase,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
  Lsc2DLoad<BlockWidth, BlockHeight, Transpose, CTL>::template run<T>(
      array, SurfaceBase,
      SurfaceWidth, SurfaceHeight, SurfacePitch,
      Src0AddrX, Src0AddrY);
}

// Intended usage:
//    lscStore<16, DataShuffle::none, CacheCtrl::L1UC_L3UC>(
//    adrs, array, 1024, 1024, 4096, 0, 0);

template <int BlockWidth, DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT,
         typename T, int BlockHeight>
static inline void lscStore(void* SurfaceBase, sycl::vec<T, BlockHeight>& array,
    int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
  Lsc2DStore<BlockWidth, BlockHeight, Transpose, CTL>::template run<T>(
      SurfaceBase, array,
      SurfaceWidth, SurfaceHeight, SurfacePitch,
      Src0AddrX, Src0AddrY);
}

template <int BlockWidth, DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT,
         typename T, int BlockHeight>
static inline void lscLoad(
    sycl::vec<T, BlockHeight>& array,
    AddressPayload<BlockWidth, BlockHeight>& address
) {
  Lsc2DLoad<BlockWidth, BlockHeight, Transpose, CTL>::template run<T>(
      array, address);
}

template <int BlockWidth, DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT,
         typename T, int BlockHeight>
static inline void lscStore(
    AddressPayload<BlockWidth, BlockHeight>& address,
    const sycl::vec<T, BlockHeight>& array
) {
  Lsc2DStore<BlockWidth, BlockHeight, Transpose, CTL>::template run<T>(
      address, array);
}
