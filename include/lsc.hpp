#pragma once

#include <type_traits>
#include <sycl/sycl.hpp>

enum DataShuffle {
  none, transpose, vnni
};

enum CacheCtrl {
  DEFAULT = 0,
  L1UC_L3UC,
  L1UC_L3WB,
  L1WT_L3UC,
  L1WT_L3WB,
  L1S_L3UC,
  L1S_L3WB,
  L1WB_L3WB
};

enum BarrierType {
  ProducerConsumer = 0,
  ProducerOnly,
  ConsumerOnly
};

#include "regmap.hpp"
#include "lsc_untyped_list.hpp"

namespace {
//
// Register allocation
//  Only OpenCL vector type could gurantee contiguous allocation
//
template <int BlockHeight, int BlockWidth,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct Lsc2DLoad {
  template <typename T> static inline void run(
      sycl::vec<T, BlockHeight>& array, void* SurfaceBase,
      int SurfaceHeight, int SurfaceWidth, int SurfacePitch,
      int Src0AddrX, int Src0Addr);
};

template <int BlockHeight, int BlockWidth,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct Lsc2DStore {
  template <typename T> static inline void run(
      void* SurfaceBase, const sycl::vec<T, BlockHeight>& array,
      int SurfaceHeight, int SurfaceWidth, int SurfacePitch,
      int Src0AddrX, int Src0Addr);
};

template <int BlockHeight, int BlockWidth,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct prefetch2D {
  template <typename T> static inline void run(
      T* SurfaceBase, int SurfaceWidth, int SurfaceHeight, int SurfacePitch,
      int Src0AddrX, int Src0AddrY);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
template <> struct prefetch2D<8, 16, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    T* SurfaceBase, int SurfaceHeight, int SurfaceWidth, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
    asm volatile ("\n"
        "lsc_load_block2d.ugm (M1_NM, 1) V0:d32.1x16x8nn flat[%1, %2, %3, %4, %5, %6]"
        :: "rw"(SurfaceBase), "rw"(SurfaceWidth), "rw"(SurfaceHeight),
        "rw"(SurfacePitch), "rw"(Src0AddrX), "rw"(Src0AddrY));
  }
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

template <int BlockWidth, DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT,
         typename T, int BlockHeight>
static inline void lscLoad(sycl::vec<T, BlockHeight>& array, void* SurfaceBase,
    int SurfaceHeight, int SurfaceWidth, int SurfacePitch,
    int Src0AddrX, int Src0AddrY) {
  Lsc2DLoad<BlockHeight, BlockWidth, Transpose, CTL>::template run<T>(
      array, SurfaceBase,
      SurfaceHeight, SurfaceWidth, SurfacePitch,
      Src0AddrX, Src0AddrY);
}

//
// lscLoad<CacheCtrl::L1UC_L3UC>(M, address);
//
template <CacheCtrl CTL= CacheCtrl::DEFAULT,
    typename T, int BlockHeight, int BlockWidth, DataShuffle Transpose>
static inline void lscLoad(
    __Matrix<T, BlockHeight, BlockWidth, Transpose>& M,
    const AddressPayload<BlockHeight, BlockWidth>& address
) {
  constexpr auto NumRegs = __Matrix<T, BlockHeight, BlockWidth, Transpose>::NumRegs;
  constexpr auto DataWidth = __Matrix<T, BlockHeight, BlockWidth, Transpose>::LSCWidth;
  RawSendLoad<DataWidth, NumRegs, Transpose, CTL>::run(M.getStorage(), address);
}

template <CacheCtrl CTL= CacheCtrl::DEFAULT,
    typename T, int BlockHeight, int BlockWidth, DataShuffle Transpose>
static inline void lscStore(
    AddressPayload<BlockHeight, BlockWidth>& address,
    const __Matrix<T, BlockHeight, BlockWidth, Transpose>& M
) {
  constexpr auto NumRegs = __Matrix<T, BlockHeight, BlockWidth, Transpose>::NumRegs;
  constexpr auto DataWidth = Log2<sizeof(T)>();
  static_assert(Transpose == DataShuffle::none, "Store support only none shuffled matrix");
  RawSendStore<DataWidth, NumRegs, Transpose, CTL>::run(address, M.getStorage());
}

template <typename T, int Height, int Width,
         DataShuffle Transpose,
         int SubGroupSize, int ArraySize>
template <CacheCtrl CTL>
inline __Matrix<T, Height, Width, Transpose, SubGroupSize, ArraySize>&
__Matrix<T, Height, Width, Transpose, SubGroupSize, ArraySize>::load(
    const AddressPayload<Height, Width, ArraySize>& address
) {
  constexpr auto NumRegs = __Matrix::NumRegs;
  constexpr auto DataWidth = __Matrix::LSCWidth;
  RawSendLoad<DataWidth, NumRegs, Transpose, CTL>::run(this->getStorage(), address);
  return *this;
}

template <typename T, int Height, int Width,
         DataShuffle Transpose,
         int SubGroupSize, int ArraySize>
template <CacheCtrl CTL>
inline __Matrix<T, Height, Width, Transpose, SubGroupSize, ArraySize>&
__Matrix<T, Height, Width, Transpose, SubGroupSize, ArraySize>::store(
    const AddressPayload<Height, Width, ArraySize>& address
) {
  constexpr auto NumRegs = __Matrix::NumRegs;
  constexpr auto DataWidth = Log2<sizeof(T)>();
  static_assert(Transpose == DataShuffle::none, "Store support only none shuffled matrix");
  RawSendStore<DataWidth, NumRegs, Transpose, CTL>::run(address, this->getStorage());
  return *this;
}

template <typename T, int Height, int Width,
         DataShuffle Transpose,
         int SubGroupSize, int ArraySize>
template <CacheCtrl CTL>
inline __ArrayMatrix<T, Height, Width, Transpose, SubGroupSize, ArraySize>&
__ArrayMatrix<T, Height, Width, Transpose, SubGroupSize, ArraySize>::load(
    const AddressPayload<Height, Width, ArraySize>& address
) {
  constexpr auto NumRegs = __ArrayMatrix::NumRegs;
  constexpr auto DataWidth = __ArrayMatrix::LSCWidth;
  RawSendLoad<DataWidth, NumRegs, Transpose, CTL>::run(this->getStorage(), address);
  return *this;
}

template <typename T, int Height, int Width,
         DataShuffle Transpose,
         int SubGroupSize, int ArraySize>
template <CacheCtrl CTL>
inline __ArrayMatrix<T, Height, Width, Transpose, SubGroupSize, ArraySize>&
__ArrayMatrix<T, Height, Width, Transpose, SubGroupSize, ArraySize>::store(
    const AddressPayload<Height, Width, ArraySize>& address
) {
  constexpr auto NumRegs = __ArrayMatrix::NumRegs;
  constexpr auto DataWidth = Log2<sizeof(T)>();
  static_assert(Transpose == DataShuffle::none, "Store support only none shuffled matrix");
  RawSendStore<DataWidth, NumRegs, Transpose, CTL>::run(address, this->getStorage());
  return *this;
}
