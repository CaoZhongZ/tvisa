#pragma once

#include "CL/cl_platform.h"
#include <sycl/sycl.hpp>

// TODO: move somewhere else
template<int N> constexpr int Log2Ceiling() {
  return Log2Ceiling<N/2>() + 1;
}

template <> constexpr int Log2Ceiling<2>() {
  return 1;
}

template <> constexpr int Log2Ceiling<1>() {
  return 1;
}

template <int N> constexpr int Log2() {
  return Log2<N/2>() + 1;
}

template <> constexpr int Log2<4>() {
  return 2;
}
template <> constexpr int Log2<2>() {
  return 1;
}
template <> constexpr int Log2<1>() {
  return 0;
}

template <int N, int L> constexpr int LowBound() {
  if constexpr (N < L) return L;
  else return N;
}

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

//
// XXX: AddressPayload is reinterpretable among configurations
//  1. Surface Pitch is in byte
//  2. Surface Width is in byte
//  3. Surface Height is in elements???
//  4. StartX/StartY and Block Width/Height are in elements???
//
template <int BlockHeight, int BlockWidth, int ArrayLength = 1>
struct AddressPayload {
  inline AddressPayload() = default;
  inline AddressPayload(
    void* SurfaceBase,
    uint32_t SurfaceHeight, uint32_t SurfaceWidth,
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

  template <int OldHeight, int OldWidth, int OldArrayLength>
  inline AddressPayload(
      const AddressPayload<OldHeight, OldWidth, OldArrayLength>& payload) :
    payloadReg_(payload.getPayload()) {
    if constexpr (OldWidth != BlockWidth || OldHeight != BlockHeight
        || OldArrayLength != ArrayLength) {
      constexpr uint32_t BWHAL = (BlockWidth -1)
        | (BlockHeight -1) << 8 | ((ArrayLength -1) & 0xf) << 16;
      asm volatile ("mov (M1,1) %0(0,7)<1> %1\n" : "+rw"(payloadReg_) : "i"(BWHAL));
    }
  }

  template <int OldHeight, int OldWidth, int OldArrayLength>
  inline AddressPayload& operator = (
      const AddressPayload<OldHeight, OldWidth, OldArrayLength>& payload) {
    payloadReg_ = payload.getPayload();
    if constexpr (OldWidth != BlockWidth || OldHeight != BlockHeight
        || OldArrayLength != ArrayLength) {
      constexpr uint32_t BWHAL = (BlockWidth -1)
        | (BlockHeight -1) << 8 | ((ArrayLength -1) & 0xf) << 16;
      asm volatile ("mov (M1,1) %0(0,7)<1> %1\n": "+rw"(payloadReg_): "i"(BWHAL));
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
        : "+rw"(payloadReg_): "rw"(addr)
    );
    return *this;
  }

  inline AddressPayload cloneAndUpdateSurfaceBase(void *addr) {
    AddressPayload newPayload (*this);
    return newPayload.upddateSurfaceBase(addr);
  }

  inline AddressPayload& updateSrc0AddrX(int x_off) {
    asm volatile ("mov (M1, 1) %0(0, 5)<1> %1(0, 0)<0;1,0>\n"
        : "+rw"(payloadReg_) : "rw"(x_off)
    );
    return *this;
  }

  inline AddressPayload& updateSrc0AddrY(int y_off) {
    asm volatile ("mov (M1, 1) %0(0, 6)<1> %1(0, 0)<0;1,0>\n"
        : "+rw"(payloadReg_) : "rw"(y_off)
    );
    return *this;
  }

private:
  uint32_t payloadReg_;
};

struct BarrierPayload {
  BarrierPayload(
      uint8_t ID,
      BarrierType type,
      uint8_t NumOfProducer,
      uint8_t NumOfConsumer) {
    uint32_t payload = ID | ((uint32_t)type) << 14
      | ((uint32_t)NumOfProducer) << 16
      | ((uint32_t)NumOfConsumer) << 24;

    asm volatile ("\n"
        "mov (M1, 1) %0(0,2)<1> %1(0,0)<0;1,0>\n"
        : "=rw"(payloadReg_) : "rw"(payload));
  }

  inline uint32_t& getPayload() {
    return payloadReg_;
  }

  inline const uint32_t& getPayload() const {
    return payloadReg_;
  }
private:
  uint32_t payloadReg_;
};

#else
template <int BlockHeight, int BlockWidth, int ArrayLength = 1>
struct AddressPayload;

struct BarrierPayload;
#endif

template <typename T, int Height, int Width,
    DataShuffle Transpose, int SubGroupSize = 16, int ArraySize = 1>
struct InnerLayout;

template <typename T, int Height, int Width, int SubGroupSize, int ArraySize>
struct InnerLayout<T, Height, Width, DataShuffle::none, SubGroupSize, ArraySize> {
private:
  static constexpr int NElemsPerLane = sizeof(int) / sizeof(T);
  static constexpr int PaddedWidth = 1 << Log2Ceiling<sizeof(T) * Width>();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
  static constexpr int PhyRegSize = 64;
  static constexpr int AllocSize = PaddedWidth * Height * ArraySize;
public:
  static constexpr int NumRegs = (AllocSize + RegSize -1) / RegSize;
  static constexpr int PhyNumRegs = (AllocSize + PhyRegSize - 1) / PhyRegSize;
  static constexpr int N = NumRegs * NElemsPerLane;
  static constexpr int LSCWidth = Log2<sizeof(T)>();
};

//
// Transpose is always on the granularity of equal or more than 4-byte
//
template <typename T, int Height, int Width, int SubGroupSize, int ArraySize>
struct InnerLayout<T, Height, Width, DataShuffle::transpose, SubGroupSize, ArraySize> {
private:
  static constexpr int NElemsPerLane = sizeof(int) / sizeof(T);
  static constexpr int PaddedHeight = LowBound<1 << Log2Ceiling<sizeof(T) * Height>(), 4 >();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
  static constexpr int AllocSize = Width * PaddedHeight * ArraySize;
  static constexpr int PhyRegSize = 64;
  static_assert(Height * sizeof(T) <= PhyRegSize, "No register space for transpose");
public:
  static constexpr int NumRegs = (AllocSize + RegSize -1)/ RegSize;
  static constexpr int PhyNumRegs = (AllocSize + PhyRegSize -1) / PhyRegSize;
  static constexpr int N = NumRegs * NElemsPerLane;
  static constexpr int LSCWidth = LowBound<Log2<sizeof(T)>(), 2>();
};

template <typename T, int Height, int Width, int SubGroupSize, int ArraySize>
struct InnerLayout<T, Height, Width, DataShuffle::vnni, SubGroupSize, ArraySize> {
private:
  static constexpr int NElemsPerLane = sizeof(int) / sizeof(T);
  static constexpr int PaddedHeight = (Height + NElemsPerLane -1)
                                      / NElemsPerLane * NElemsPerLane;
  // We eliminate the case implicit operation is carried.
  static_assert(Height == PaddedHeight, "Error: HeightxElemsize must align to 4 bytes");
  static constexpr int PaddedWidth = 1 << Log2Ceiling<sizeof(T) * Width>();
  static constexpr int RegSize = SubGroupSize * sizeof(int);
  static constexpr int AllocSize = PaddedWidth * PaddedHeight * ArraySize;
  static constexpr int PhyRegSize = 64;
  static_assert(Width * sizeof(int) <= PhyRegSize, "No register space for VNNI transform");
public:
  static constexpr int NumRegs = (AllocSize + RegSize -1) / RegSize;
  static constexpr int PhyNumRegs = (AllocSize + PhyRegSize -1) / PhyRegSize;
  static constexpr int N = NumRegs * NElemsPerLane;
  static constexpr int LSCWidth = Log2<sizeof(T)>();
  static_assert(LSCWidth < 2, "VNNI only makes sense on less than 16-bit");
};

//
// memory region represents as regsiter image
//
template <typename T, int Height, int Width,
         DataShuffle Transpose = DataShuffle::none,
         int SubGroupSize = 16, int ArraySize = 1>
struct __Matrix {
  using layout = InnerLayout<T, Height, Width, Transpose, SubGroupSize, ArraySize>;
  static constexpr int PhyNumRegs = layout::PhyNumRegs;
  static constexpr int NumRegs = layout::NumRegs;
  static constexpr int N = layout::N;
  static constexpr int LSCWidth = layout::LSCWidth;

  static_assert(sizeof(sycl::vec<T, N>)/sizeof(uint32_t) == NumRegs);
  using rawType = sycl::vec<uint32_t, sizeof(sycl::vec<T, N>)/sizeof(uint32_t)>;

  inline typename sycl::vec<T, N>::vector_t& getStorage() {
    return reinterpret_cast<typename sycl::vec<T, N>::vector_t&>(registerImage_);
  }
  inline const typename sycl::vec<T, N>::vector_t& getStorage() const {
    return reinterpret_cast<const typename sycl::vec<T, N>::vector_t&>(registerImage_);
  }
  inline typename sycl::vec<T, N>& getImage() {
    return registerImage_;
  }
  inline const typename sycl::vec<T, N>& getImage() const {
    return registerImage_;
  }

  /* XXX: Generate unecessary copy, WTF??? */
  // inline typename rawType::vector_t& getRawStorage() {
  //   return reinterpret_cast<typename rawType::vector_t&>(registerImage_);
  // }
  // inline const typename rawType::vector_t& getRawStorage() const {
  //   return reinterpret_cast<const typename rawType::vector_t&>(registerImage_);
  // }

  __Matrix() = default;
  __Matrix(const sycl::vec<T, N>& rh) : registerImage_(rh) {}
  __Matrix(sycl::vec<T, N>&& rh) : registerImage_(std::move(rh)) {}
  __Matrix(const __Matrix &rh) : registerImage_(rh.registerImage_) {}
  __Matrix(__Matrix&& rh) : registerImage_(std::move(rh.registerImage_)) {}
  
  template<typename SrcT>
  __Matrix(const __Matrix<SrcT, Height, Width, Transpose, SubGroupSize, ArraySize> &rh) {
    #pragma  unroll
    for(int i=0; i<N; ++i)
      registerImage_[i] = static_cast<T>((rh.getImage())[i]);
  }

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
  inline __Matrix& load(const AddressPayload<Height, Width, ArraySize> &address);

  template <CacheCtrl CTL = CacheCtrl::DEFAULT>
  inline __Matrix& store(const AddressPayload<Height, Width, ArraySize> &address);

  // Review as re-shuffled tensor in logic view, need careful review here.
  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::vnni
  && reshuffle == DataShuffle::none,
    __Matrix<T, Height / 2, Width * 2, reshuffle, SubGroupSize, ArraySize>
      >::type cast () {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::transpose
  && reshuffle == DataShuffle::none,
    __Matrix<T, Width, Height, reshuffle, SubGroupSize, ArraySize>
      >::type cast() {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::none
  && reshuffle == DataShuffle::vnni,
    __Matrix<T, Height * 2, Width /2, reshuffle, SubGroupSize, ArraySize>
      >::type cast() {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::none
  && reshuffle == DataShuffle::transpose,
    __Matrix<T, Width, Height, DataShuffle::transpose, SubGroupSize, ArraySize>
      >::type cast() {
    return {registerImage_};
  }

  //
  // XXX: performance problem for SIMD16/sycl::half when compiler generate
  // two instructions for single register operation.
  //
  inline void zero() {
    registerImage_ = 0;
  }

private:
  sycl::vec<T, N> registerImage_;
};

//
// Test for normal array type instead of vector type
//
template <typename T, int Height, int Width,
         DataShuffle Transpose = DataShuffle::none,
         int SubGroupSize = 16, int ArraySize = 1>
struct __ArrayMatrix {
  using layout = InnerLayout<T, Height, Width, Transpose, SubGroupSize, ArraySize>;
  static constexpr int PhyNumRegs = layout::PhyNumRegs;  
  static constexpr int NumRegs = layout::NumRegs;
  static constexpr int N = layout::N;
  static constexpr int LSCWidth = layout::LSCWidth;

  static_assert(NumRegs == sizeof(T) * N/sizeof(uint32_t));

  /* XXX: Generate unecessary copy, WTF???
  inline typename sycl::vec<T, N>::vector_t& getStorage() {
    return reinterpret_cast<typename sycl::vec<T, N>::vector_t&>(registerImage_);
  }
  inline const typename sycl::vec<T, N>::vector_t& getStorage() const {
    return reinterpret_cast<const typename sycl::vec<T, N>::vector_t&>(registerImage_);
  }
  */

  typedef __attribute__((ext_vector_type(N))) T storage_type;
  inline storage_type& getStorage() {
    return reinterpret_cast<storage_type&>(registerImage_);
  }
  inline const storage_type& getStorage() const {
    return reinterpret_cast<const storage_type&>(registerImage_);
  }

  __ArrayMatrix() = default;
  __ArrayMatrix(const storage_type& rh): registerImage_(rh) {}
  __ArrayMatrix(storage_type&& rh): registerImage_(std::move(rh)) {}
  __ArrayMatrix(const __ArrayMatrix &rh) : registerImage_(rh.registerImage_) {}

  inline __ArrayMatrix& operator = (const __ArrayMatrix& rh) {
    registerImage_ = std::move(rh.registerImage_);    
    return *this;
  }

  inline __ArrayMatrix operator + (const __ArrayMatrix& m) {
    return { registerImage_ + m.registerImage_ };
  }
  inline __ArrayMatrix operator - (const __ArrayMatrix& m) {
    return { registerImage_ - m.registerImage_ };
  }
  inline __ArrayMatrix operator * (const __ArrayMatrix& m) {
    return { registerImage_ * m.registerImage_ };
  }
  inline __ArrayMatrix operator / (const __ArrayMatrix& m) {
    return { registerImage_ * m.registerImage_ };
  }

  template <CacheCtrl CTL = CacheCtrl::DEFAULT>
  inline __ArrayMatrix& load(const AddressPayload<Height, Width, ArraySize> &address);

  template <CacheCtrl CTL = CacheCtrl::DEFAULT>
  inline __ArrayMatrix& store(const AddressPayload<Height, Width, ArraySize> &address);

  // Review as re-shuffled tensor in logic view, need careful review here.
  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::vnni
  && reshuffle == DataShuffle::none,
    __ArrayMatrix<T, Height / 2, Width * 2, reshuffle, SubGroupSize, ArraySize>
      >::type cast () {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::transpose
  && reshuffle == DataShuffle::none,
    __ArrayMatrix<T, Width, Height, reshuffle, SubGroupSize, ArraySize>
      >::type cast() {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::none
  && reshuffle == DataShuffle::vnni,
    __ArrayMatrix<T, Height * 2, Width /2, reshuffle, SubGroupSize, ArraySize>
      >::type cast() {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::none
  && reshuffle == DataShuffle::transpose,
    __ArrayMatrix<T, Width, Height, DataShuffle::transpose, SubGroupSize, ArraySize>
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
  storage_type registerImage_;
};

// __RawMatrix for workaround IGC dpas type requirement if alias doesn't work
template <typename T, int Height, int Width,
         DataShuffle Transpose = DataShuffle::none,
         int SubGroupSize = 16, int ArraySize = 1>
struct __RawMatrix {
  using layout = InnerLayout<T, Height, Width, Transpose, SubGroupSize, ArraySize>;
  static constexpr int NumRegs = layout::NumRegs;
  static constexpr int N = layout::N;

  static_assert(NumRegs == sizeof(sycl::vec<T, N>)/sizeof(uint32_t));
  using rawType = sycl::vec<uint32_t, sizeof(sycl::vec<T, N>)/sizeof(uint32_t)>;

  /* XXX: Generate unecessary copy, WTF???
  inline typename sycl::vec<T, N>::vector_t& getStorage() {
    return reinterpret_cast<typename sycl::vec<T, N>::vector_t&>(registerImage_);
  }
  inline const typename sycl::vec<T, N>::vector_t& getStorage() const {
    return reinterpret_cast<const typename sycl::vec<T, N>::vector_t&>(registerImage_);
  }
  */

  inline typename rawType::vector_t& getStorage() {
    return reinterpret_cast<typename rawType::vector_t&>(registerImage_);
  }
  inline const typename rawType::vector_t& getStorage() const {
    return reinterpret_cast<const typename rawType::vector_t&>(registerImage_);
  }

  __RawMatrix() = default;
  __RawMatrix(const sycl::vec<T, N>& rh) : registerImage_(rh) {}
  __RawMatrix(sycl::vec<T, N>&& rh) : registerImage_(std::move(rh)) {}
  __RawMatrix(const __RawMatrix &rh) : registerImage_(rh.registerImage_) {}
  __RawMatrix(__RawMatrix&& rh) : registerImage_(std::move(rh.registerImage_)) {}

  inline __RawMatrix& operator = (__RawMatrix&& rh) {
    registerImage_ = std::move(rh.registerImage_);
    return *this;
  }
  inline __RawMatrix& operator = (const __RawMatrix& rh) {
    registerImage_ = rh.registerImage_;
    return *this;
  }
  inline __RawMatrix operator + (const __RawMatrix& m) {
    return { registerImage_ + m.registerImage_ };
  }
  inline __RawMatrix operator - (const __RawMatrix& m) {
    return { registerImage_ - m.registerImage_ };
  }
  inline __RawMatrix operator * (const __RawMatrix& m) {
    return { registerImage_ * m.registerImage_ };
  }
  inline __RawMatrix operator / (const __RawMatrix& m) {
    return { registerImage_ * m.registerImage_ };
  }

  template <CacheCtrl CTL = CacheCtrl::DEFAULT>
  inline __RawMatrix& load(const AddressPayload<Height, Width, ArraySize> &address);

  template <CacheCtrl CTL = CacheCtrl::DEFAULT>
  inline __RawMatrix& store(const AddressPayload<Height, Width, ArraySize> &address);

  // Review as re-shuffled tensor in logic view, need careful review here.
  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::vnni
  && reshuffle == DataShuffle::none,
    __RawMatrix<T, Height / 2, Width * 2, reshuffle, SubGroupSize, ArraySize>
      >::type cast () {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::transpose
  && reshuffle == DataShuffle::none,
    __RawMatrix<T, Width, Height, reshuffle, SubGroupSize, ArraySize>
      >::type cast() {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::none
  && reshuffle == DataShuffle::vnni,
    __RawMatrix<T, Height * 2, Width /2, reshuffle, SubGroupSize, ArraySize>
      >::type cast() {
    return {registerImage_};
  }

  template <DataShuffle reshuffle>
  inline typename std::enable_if<Transpose == DataShuffle::none
  && reshuffle == DataShuffle::transpose,
    __RawMatrix<T, Width, Height, DataShuffle::transpose, SubGroupSize, ArraySize>
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
  rawType registerImage_;
};
