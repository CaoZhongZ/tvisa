#pragma once

#include <sycl/sycl.hpp>

enum DataShuffle {
  none, transpose, vnni
};

// TODO: move somewhere else
template<int N> constexpr int Log2() {
  return Log2<N/2>() + 1;
}

template <> constexpr int Log2<2>() {
  return 1;
};

template <> constexpr int Log2<1>() {
  return 1;
}

template <int N, int L> constexpr int LowBound() {
  if constexpr (N < L) return L;
  else return N;
}

template <typename T, int Width, int Height,
    DataShuffle Transpose, int ArraySize = 1, int SubGroupSize = 16>
struct InnerLayout;

template <typename T, int Width, int Height,
         int ArraySize = 1, int SubGroupSize = 16>
struct InnerLayout<T, Width, Height, DataShuffle::none, ArraySize, SubGroupSize> {
private:
  static constexpr int PaddedWidth = 1 << Log2<sizeof(T) * Width>();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
public:
  static constexpr int NumRegs = PaddedWidth * Height / RegSize * ArraySize;
  static constexpr int N = NumRegs * sizeof(int)/sizeof(T);
}

template <typename T, int Width, int Height,
         int ArraySize = 1, int SubGroupSize = 16>
struct InnerLayout<T, Width, Height, DataShuffle::transpose, ArraySize, SubGroupSize> {
private:
  static constexpr int PaddedHeight = LowBound<1 << Log2<sizeof(T) * Height>(), 4 >();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
public:
  static constexpr int NumRegs = Width * PaddedHeight / RegSize * ArraySize;
  static constexpr int N = NumRegs * sizeof(int)/sizeof(T);
}

template <typename T, int Width, int Height,
         int ArraySize = 1, int SubGroupSize = 16>
struct InnerLayout<T, Width, Height, DataShuffle::vnni, ArraySize, SubGroupSize> {
private:
  static constexpr int NElemsPerLane = sizeof(int) / sizeof(T);
  static constexpr int PaddedHeight = (Height + ElemsPerLane -1) / ElemsPerLane;
  static constexpr int PaddedWidth = 1 << Log2<sizeof(T) * Width>();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
public:
  static constexpr int NumRegs = PaddedWidth * PaddedHeight / RegSize * ArraySize;
  static constexpr int N = NumRegs * sizeof(int)/sizeof(T);
}

//
// memory region represents as regsiter image
//
template <typename T, int Width, int Height,
         DataShuffle Transpose = DataShuffle::none,
         int ArraySize = 1, int SubGroupSize = 16>
struct __Matrix {
  using layout = InnerLayout<T, Width, Height, Transpose, ArraySize, SubGroupSize>;
  constexpr int NumRegs = layout::NumRegs;
  constexpr int N = layout::N;

  sycl::vec<T, N> registerImage_;
};

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
    return *this;
  }

  inline uint32_t& getPayload() {
    return payloadReg_;
  }

  inline const uint32_t& getPayload() const {
    return payloadReg_;
  }

  inline AddressPayload& addSrc0AddrX(int offset) {
    asm volatile (
        "add (M1, 1) %0(0, 5)<1> %0(0, 5)<0;1,0> %1(0,0)<0;1,0>\n"
        : "+rw"(payloadReg_) : "rw"(offset)
    );
    return *this;
  }

  inline AddressPayload& addSrc0AddrY(int offset) {
    asm volatile (
        "add (M1, 1) %0(0, 6)<1> %0(0, 6)<0;1,0> %1(0,0)<0;1,0>\n"
        : "+rw"(payloadReg_) : "rw"(offset)
    );
    return *this;
  }

  inline AddressPayload& updateSurfaceBase(void *addr) {
    asm volatile ("{\n"
        ".decl alias64 v_type=G type=uq num_elts=8 align=GRF alias=<%0, 0>\n"
        "mov (M1, 1) alias64(0, 0)<1> %1(0, 0)<0;1,0>\n"
        "}\n"
        : "+rw"(payloadReg_) : "rw"(addr)
    );
    return *this;
  }

  inline AddressPayload cloneAndUpdateSurfaceBase(void *addr) {
    AddressPayload newPayload (*this);
    return newPayload.upddateSurfaceBase(addr);
  }

  inline AddressPayload& updateSrc0AddrX(int x_off) {
    asm volatile ("{\n"
        "mov (M1, 1) %0(0, 5)<1> %1(0, 0)<0;1,0>\n"
        "}\n"
        : "=rw"(payloadReg_) : "rw"(x_off)
    );
    return *this;
  }

  inline AddressPayload& updateSrc0AddrY(int y_off) {
    asm volatile ("{\n"
        "mov (M1, 1) %0(0, 6)<1> %1(0, 0)<0;1,0>\n"
        "}\n"
        : "=rw"(payloadReg_) : "rw"(y_off)
    );
    return *this;
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

#else
template <int BlockWidth, int BlockHeight, int ArrayLength = 1>
struct AddressPayload;
#endif
