#pragma once

#include <sycl/sycl.hpp>

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

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

template <int BlockWidth, int BlockHeight, int ArrayLength = 1>
struct AddressPayload {
  inline AddressPayload() = default;
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
        : "=rw"(payloadReg_) : "rw"(SurfaceBase), "rw"(SurfaceWidth),
        "rw"(SurfaceHeight), "rw"(SurfacePitch), "rw"(Src0AddrX),
        "rw"(Src0AddrY), "i"(BWHAL)
        /*
        : "+rw"(payloadReg_) : "rw.u"(SurfaceBase), "rw.u"(SurfaceWidth),
        "rw.u"(SurfaceHeight), "rw.u"(SurfacePitch), "rw.u"(Src0AddrX),
        "rw.u"(Src0AddrY), "i"(BWHAL)
        */
    );
  }

  template <int OldWidth, int OldHeight, int OldArrayLength>
  inline AddressPayload(
      const AddressPayload<OldWidth, OldHeight, OldArrayLength>& payload) :
    payloadReg_(payload.getPayload()) {
    if constexpr (OldWidth != BlockWidth || OldHeight != BlockHeight
        || OldArrayLength != ArrayLength) {
      constexpr uint32_t BWHAL = (BlockWidth -1)
        | (BlockHeight -1) << 8 | ((ArrayLength -1) & 0xf) << 16;
      asm volatile ("mov (M1,1) %0(0,7)<1> %1\n" : "=rw"(payloadReg_) : "i"(BWHAL));
    }
  }

  template <int OldWidth, int OldHeight, int OldArrayLength>
  inline AddressPayload& operator = (
      const AddressPayload<OldWidth, OldHeight, OldArrayLength>& payload) {
    payloadReg_ = payload.getPayload();
    if constexpr (OldWidth != BlockWidth || OldHeight != BlockHeight
        || OldArrayLength != ArrayLength) {
      constexpr uint32_t BWHAL = (BlockWidth -1)
        | (BlockHeight -1) << 8 | ((ArrayLength -1) & 0xf) << 16;
      asm volatile ("mov (M1,1) %0(0,7)<1> %1\n": "=rw"(payloadReg_): "i"(BWHAL));
    }
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

  // XXX: potential problem in final code! IGC crash on DESSA pass
  inline AddressPayload& updateSurfaceBase(void *addr) {
    asm volatile ("{\n"
        ".decl alias64 v_type=G type=uq num_elts=8 align=GRF alias=<%0, 0>\n"
        "mov (M1,1) alias64(0, 0)<1> %1(0, 0)<0;1,0>\n"
        // "mov (M1,16) %0(0,0)<1> %0(0,0)<1;1,0>\n"
        "}\n"
        : :"rw"(payloadReg_), "rw"(addr)
    );
    return *this;
  }

  inline AddressPayload cloneAndUpdateSurfaceBase(void *addr) {
    AddressPayload newPayload (*this);
    return newPayload.upddateSurfaceBase(addr);
  }

  inline AddressPayload& updateSrc0AddrX(int x_off) {
    asm volatile ("mov (M1, 1) %0(0, 5)<1> %1(0, 0)<0;1,0>\n"
        : "=rw"(payloadReg_) : "rw"(x_off)
    );
    return *this;
  }

  inline AddressPayload& updateSrc0AddrY(int y_off) {
    asm volatile ("mov (M1, 1) %0(0, 6)<1> %1(0, 0)<0;1,0>\n"
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
      // "mov (M1, 16) %0(0, 0)<1> %0(0, 0)<1;1,0>\n"
      "}\n"
      : :"rw"(addressPayload), "rw"(base)
  );
  return addressPayload;
}

#else
template <int BlockWidth, int BlockHeight, int ArrayLength = 1>
struct AddressPayload;
#endif

template <typename T, int Width, int Height,
    DataShuffle Transpose, int ArraySize = 1, int SubGroupSize = 16>
struct InnerLayout;

template <typename T, int Width, int Height, int ArraySize, int SubGroupSize>
struct InnerLayout<T, Width, Height, DataShuffle::none, ArraySize, SubGroupSize> {
private:
  static constexpr int PaddedWidth = 1 << Log2<sizeof(T) * Width>();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
  static constexpr int AllocSize = PaddedWidth * Height * ArraySize;
public:
  static constexpr int NumRegs = (AllocSize + RegSize -1) / RegSize;
  static constexpr int N = NumRegs * sizeof(int)/sizeof(T);
};

template <typename T, int Width, int Height, int ArraySize, int SubGroupSize>
struct InnerLayout<T, Width, Height, DataShuffle::transpose, ArraySize, SubGroupSize> {
private:
  static constexpr int PaddedHeight = LowBound<1 << Log2<sizeof(T) * Height>(), 4 >();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
  static constexpr int AllocSize = Width * PaddedHeight * ArraySize;
public:
  static constexpr int NumRegs = (AllocSize + RegSize -1)/ RegSize;
  static constexpr int N = NumRegs * sizeof(int)/sizeof(T);
};

template <typename T, int Width, int Height, int ArraySize, int SubGroupSize>
struct InnerLayout<T, Width, Height, DataShuffle::vnni, ArraySize, SubGroupSize> {
private:
  static constexpr int NElemsPerLane = sizeof(int) / sizeof(T);
  static constexpr int PaddedHeight = (Height + NElemsPerLane -1) / NElemsPerLane;
  static constexpr int PaddedWidth = 1 << Log2<sizeof(T) * Width>();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
  static constexpr int AllocSize = PaddedWidth * PaddedHeight * ArraySize;
public:
  static constexpr int NumRegs = (AllocSize + RegSize -1) / RegSize;
  static constexpr int N = NumRegs * sizeof(int)/sizeof(T);
};

//
// memory region represents as regsiter image
//
template <typename T, int Width, int Height,
         DataShuffle Transpose = DataShuffle::none,
         int ArraySize = 1, int SubGroupSize = 16>
struct __Matrix {
  using layout = InnerLayout<T, Width, Height, Transpose, ArraySize, SubGroupSize>;
  static constexpr int NumRegs = layout::NumRegs;
  static constexpr int N = layout::N;

  using rawType = sycl::vec<uint32_t, sizeof(sycl::vec<T, N>)/sizeof(uint32_t)>;

  inline typename sycl::vec<T, N>::vector_t& getStorage() {
    return reinterpret_cast<typename sycl::vec<T, N>::vector_t&>(registerImage_);
  }
  inline const typename sycl::vec<T, N>::vector_t& getStorage() const {
    return reinterpret_cast<const typename sycl::vec<T, N>::vector_t&>(registerImage_);
  }

  inline typename rawType::vector_t& getRawStorage() {
    return reinterpret_cast<typename rawType::vector_t&>(registerImage_);
  }
  inline const typename rawType::vector_t& getRawStorage() const {
    return reinterpret_cast<const typename rawType::vector_t&>(registerImage_);
  }

  __Matrix() = default;
  __Matrix(const sycl::vec<T, N>& rh) : registerImage_(rh) {}
  __Matrix(sycl::vec<T, N>&& rh) : registerImage_(std::move(rh)) {}
  __Matrix(const __Matrix &rh) : registerImage_(rh.registerImage_) {}
  __Matrix(__Matrix&& rh) : registerImage_(std::move(rh.registerImage_)) {}

  inline __Matrix& operator = (__Matrix&& rh) {
    registerImage_ = std::move(rh.registerImage_);
    return *this;
  }
  inline __Matrix& operator = (const __Matrix& rh) {
    registerImage_ = rh.registerImage_;
    return *this;
  }
  inline __Matrix operator + (const __Matrix& m) {
    return { registerImage_ + m.registerImage_ };
  }
  inline __Matrix operator - (const __Matrix& m) {
    return { registerImage_ - m.registerImage_ };
  }
  inline __Matrix operator * (const __Matrix& m) {
    return { registerImage_ * m.registerImage_ };
  }
  inline __Matrix operator / (const __Matrix& m) {
    return { registerImage_ * m.registerImage_ };
  }

  template <CacheCtrl CTL = CacheCtrl::DEFAULT>
  inline __Matrix& load(const AddressPayload<Width, Height, ArraySize> &address);

  template <CacheCtrl CTL = CacheCtrl::DEFAULT>
  inline __Matrix& store(const AddressPayload<Width, Height, ArraySize> &address);

  // Review as re-shuffled tensor in logic view, need careful review here.
  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::vnni
  && reshuffle == DataShuffle::none,
    __Matrix<T, Width * 2, Height / 2, reshuffle, ArraySize, SubGroupSize>
      >::type cast () {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::transpose
  && reshuffle == DataShuffle::none,
    __Matrix<T, Height, Width, reshuffle, ArraySize, SubGroupSize>
      >::type cast() {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::none
  && reshuffle == DataShuffle::vnni,
    __Matrix<T, Width /2, Height * 2, reshuffle, ArraySize, SubGroupSize>
      >::type cast() {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::none
  && reshuffle == DataShuffle::transpose,
    __Matrix<T, Height, Width, DataShuffle::transpose, ArraySize, SubGroupSize>
      >::type cast() {
    return {registerImage_};
  }

  //
  // XXX: performance problem for SIMD16/sycl::half when compiler generate 
  // two instructions for single register operation.
  //
  inline void zero() {
    registerImage_ =0;
  }

private:
  sycl::vec<T, N> registerImage_;
};
