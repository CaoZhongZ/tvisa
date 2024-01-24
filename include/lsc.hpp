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

#include "regmap.hpp"
#include "lsc_untyped_list.hpp"

namespace {
//
// Register allocation
//  Only OpenCL vector type could gurantee contiguous allocation
//
template <int BlockWidth, int BlockHeight,
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

template <int BlockWidth, int BlockHeight,
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

#endif
} // enumrate space

// more load/store instruction enumeration
namespace {

template <int BlockWidth, int BlockHeight,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct OptLsc2DLoad {
  template <typename T> static inline void run(
      sycl::vec<T, BlockHeight>& array,
      AddressPayload<BlockWidth, BlockHeight>& address);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
//[(<P>)] {raw_sends|raw_sendsc}[_eot] <sfid> <numSrc0> <numSrc1> <numDst> (<exec_size>) <ExMsgDesc> <Desc> <Src0> <Src1> <Dst>

//
// XXX: Only 32-bit type with shape of Nx16 could match register region in exact.
// Other shapes and format should resort to __Matrix interface.
//
template <>
struct OptLsc2DLoad<16, 8, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, AddressPayload<16, 8>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.8 (M1, 1) 0x0:ud 0x2800403:ud %1.0 V0.0 %0.0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};

template <>
struct OptLsc2DLoad<16, 16, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    sycl::vec<T, 16>& array, AddressPayload<16, 16>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.16 (M1, 1) 0x0:ud 0x3000403:ud %1.0 V0.0 %0.0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 16>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};

template <>
struct OptLsc2DLoad<16, 8, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, AddressPayload<16, 8>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.8 (M1, 1) 0x0:ud 0x2820403:ud %1.0 V0.0 %0.0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 8>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};

template <>
struct OptLsc2DLoad<16, 16, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    sycl::vec<T, 16>& array, AddressPayload<16, 16>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.16 (M1, 1) 0x0:ud 0x3020403:ud %1.0 V0.0 %0.0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 16>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};

template <>
struct OptLsc2DLoad<16, 4, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, AddressPayload<16, 4>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.4 (M1, 1) 0x0:ud 0x2400403:ud %1.0 V0.0 %0.0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 4>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};

template <>
struct OptLsc2DLoad<16, 4, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
    sycl::vec<T, 8>& array, AddressPayload<16, 4>& address
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.0.4 (M1, 1) 0x0:ud 0x2420403:ud %1.0 V0.0 %0.0\n"
        : "=rw"(reinterpret_cast<typename sycl::vec<T, 4>::vector_t&>(array))
        : "rw"(address.getPayload()));
  }
};
#endif

template <int BlockWidth, int BlockHeight,
         DataShuffle Transpose, CacheCtrl = CacheCtrl::DEFAULT>
struct OptLsc2DStore {
  template <typename T> static inline void run(
      AddressPayload<BlockWidth, BlockHeight>& address,
      const sycl::vec<T, BlockHeight>& array);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

template <>
struct OptLsc2DStore<16, 8, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
      AddressPayload<16, 8>& address, const sycl::vec<T, 8>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.8.0 (M1, 1) 0x0:ud 0x2800407:ud %0.0 %1.0 V0.0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<const typename sycl::vec<T, 8>::vector_t&>(array))
    );
  }
};

template <>
struct OptLsc2DStore<16, 16, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
      AddressPayload<16, 16>& address, const sycl::vec<T, 16>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.16.0 (M1, 1) 0x0:ud 0x3000407:ud %0.0 %1.0 V0.0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<const typename sycl::vec<T, 16>::vector_t&>(array))
    );
  }
};

template <>
struct OptLsc2DStore<16, 8, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
      AddressPayload<16, 8>& address, const sycl::vec<T, 8>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.8.0 (M1, 1) 0x0:ud 0x2820407:ud %0.0 %1.0 V0.0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<const typename sycl::vec<T, 8>::vector_t&>(array))
    );
  }
};

template <>
struct OptLsc2DStore<16, 16, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
      AddressPayload<16, 16>& address, const sycl::vec<T, 16>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.16.0 (M1, 1) 0x0:ud 0x3020407:ud %0.0 %1.0 V0.0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<const typename sycl::vec<T, 16>::vector_t&>(array))
    );
  }
};

template <>
struct OptLsc2DStore<16, 4, DataShuffle::none, CacheCtrl::DEFAULT> {
  template <typename T> static inline void run(
      AddressPayload<16, 4>& address, const sycl::vec<T, 4>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.4.0 (M1, 1) 0x0:ud 0x2400407:ud %0.0 %1.0 V0.0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<const typename sycl::vec<T, 4>::vector_t&>(array))
    );
  }
};

template <>
struct OptLsc2DStore<16, 4, DataShuffle::none, CacheCtrl::L1UC_L3UC> {
  template <typename T> static inline void run(
      AddressPayload<16, 4>& address, const sycl::vec<T, 4>& array
  ) {
    asm volatile ("\n"
        "raw_sends.15.1.4.0 (M1, 1) 0x0:ud 0x2420407:ud %0.0 %1.0 V0.0\n" ::
        "rw"(address.getPayload()),
        "rw"(reinterpret_cast<const typename sycl::vec<T, 4>::vector_t&>(array))
    );
  }
};

#endif

/*
template <int BlockWidth, int BlockHeight,
         DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT>
struct OptLsc2DLoad {
  template <typename T> static inline void run(
      __Matrix<T, BlockWidth, BlockHeight, Transpose>& M,
      AddressPayload<BlockWidth, BlockHeight>& address) {
    constexpr auto NumRegs = M.NumRegs;
    constexpr auto DataWidth = Log2<sizeof(T)>();
    RawSendLoad<DataWidth, NumRegs, Transpose, CTL>::run(M.getStorage(), address);
  }
};

template <int BlockWidth, int BlockHeight,
         DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT>
struct OptLsc2DStore {
  template <typename T> static inline void run(
      AddressPayload<BlockWidth, BlockHeight>& address,
      __Matrix<T, BlockWidth, BlockHeight, Transpose>& M) {
    constexpr auto NumRegs = M.NumRegs;
    constexpr auto DataWidth = Log2<sizeof(T)>();
    RawSendStore<DataWidth, NumRegs, Transpose, CTL>::run(address, M.getStorage());
  }
};
*/

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
  OptLsc2DLoad<BlockWidth, BlockHeight, Transpose, CTL>::template run<T>(
      array, address);
}

template <int BlockWidth, DataShuffle Transpose, CacheCtrl CTL= CacheCtrl::DEFAULT,
         typename T, int BlockHeight>
static inline void lscStore(
    AddressPayload<BlockWidth, BlockHeight>& address,
    const sycl::vec<T, BlockHeight>& array
) {
  OptLsc2DStore<BlockWidth, BlockHeight, Transpose, CTL>::template run<T>(
      address, array);
}

//
// lscLoad<CacheCtrl::L1UC_L3UC>(M, address);
//
template <CacheCtrl CTL= CacheCtrl::DEFAULT,
    typename T, int BlockWidth, int BlockHeight, DataShuffle Transpose>
static inline void lscLoad(
    __Matrix<T, BlockWidth, BlockHeight, Transpose>& M,
    const AddressPayload<BlockWidth, BlockHeight>& address
) {
  constexpr auto NumRegs = __Matrix<T, BlockWidth, BlockHeight, Transpose>::NumRegs;
  constexpr auto DataWidth = Log2<sizeof(T)>();
  RawSendLoad<DataWidth, NumRegs, Transpose, CTL>::run(M.getStorage(), address);
}

template <CacheCtrl CTL= CacheCtrl::DEFAULT,
    typename T, int BlockWidth, int BlockHeight, DataShuffle Transpose>
static inline void lscStore(
    AddressPayload<BlockWidth, BlockHeight>& address,
    const __Matrix<T, BlockWidth, BlockHeight, Transpose>& M
) {
  constexpr auto NumRegs = __Matrix<T, BlockWidth, BlockHeight, Transpose>::NumRegs;
  constexpr auto DataWidth = Log2<sizeof(T)>();
  RawSendStore<DataWidth, NumRegs, Transpose, CTL>::run(address, M.getStorage());
}

template <typename T, int Width, int Height,
         DataShuffle Transpose,
         int ArraySize, int SubGroupSize>
template <CacheCtrl CTL>
inline __Matrix<T, Width, Height, Transpose, ArraySize, SubGroupSize>&
__Matrix<T, Width, Height, Transpose, ArraySize, SubGroupSize>::load(
    const AddressPayload<Width, Height, ArraySize>& address
) {
  constexpr auto NumRegs = __Matrix::NumRegs;
  constexpr auto DataWidth = Log2<sizeof(T)>();
  RawSendLoad<DataWidth, NumRegs, Transpose, CTL>::run(this->getStorage(), address);
  return *this;
}

template <typename T, int Width, int Height,
         DataShuffle Transpose,
         int ArraySize, int SubGroupSize>
template <CacheCtrl CTL>
inline __Matrix<T, Width, Height, Transpose, ArraySize, SubGroupSize>&
__Matrix<T, Width, Height, Transpose, ArraySize, SubGroupSize>::store(
    const AddressPayload<Width, Height, ArraySize>& address
) {
  constexpr auto NumRegs = __Matrix::NumRegs;
  constexpr auto DataWidth = Log2<sizeof(T)>();
  RawSendStore<DataWidth, NumRegs, Transpose, CTL>::run(address, this->getStorage());
  return *this;
}
