#pragma once

template <typename T, int N> struct SetZero;

#if defined(__SYCL_DEVICE_ONLY__)    

#define CONCAT(A, B) _CONCAT(A, B)
#define _CONCAT(A, B) A##B

#define _MOV1 "mov (M1, 16) alias_(0, 0)<1> 0:ud\n"
#define _MOV2 _MOV1 "mov (M1, 16) alias_(1, 0)<1> 0:ud\n"

#define _MOV4                                                   \
  _MOV2 "mov (M1, 16) alias_(2, 0)<1> 0:ud\n"          \
        "mov (M1, 16) alias_(3, 0)<1> 0:ud\n"

#define _MOV8                                                   \
  _MOV4 "mov (M1, 16) alias_(4, 0)<1> 0:ud\n"          \
        "mov (M1, 16) alias_(5, 0)<1> 0:ud\n"          \
        "mov (M1, 16) alias_(6, 0)<1> 0:ud\n"          \
        "mov (M1, 16) alias_(7, 0)<1> 0:ud\n"

#define _MOV16                                                  \
  _MOV8 "mov (M1, 16) alias_(8, 0)<1> 0:ud\n"          \
        "mov (M1, 16) alias_(9, 0)<1> 0:ud\n"          \
        "mov (M1, 16) alias_(10, 0)<1> 0:ud\n"         \
        "mov (M1, 16) alias_(11, 0)<1> 0:ud\n"         \
        "mov (M1, 16) alias_(12, 0)<1> 0:ud\n"         \
        "mov (M1, 16) alias_(13, 0)<1> 0:ud\n"         \
        "mov (M1, 16) alias_(14, 0)<1> 0:ud\n"         \
        "mov (M1, 16) alias_(15, 0)<1> 0:ud\n"
        
#define _MOV32                                                  \
  _MOV16 "mov (M1, 16) alias_(16, 0)<1> 0:ud\n"          \
         "mov (M1, 16) alias_(17, 0)<1> 0:ud\n"          \
         "mov (M1, 16) alias_(18, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(19, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(20, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(21, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(22, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(23, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(24, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(25, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(26, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(27, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(28, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(29, 0)<1> 0:ud\n"         \
         "mov (M1, 16) alias_(30, 0)<1> 0:ud\n"        \
         "mov (M1, 16) alias_(31, 0)<1> 0:ud\n"                    

template <typename T> struct SetZero<T, 4> {
  static inline void run(T &target) {
    asm volatile("{\n"
                  ".decl alias_ v_type=G type=ud num_elts=64 align=GRF alias=<%0, 0>\n"
                  _MOV4
                  "}\n"
                 : "=rw"(target));
  }
};

template <typename T> struct SetZero<T, 8> {
  static inline void run(T &target) {
    asm volatile("{\n"
                ".decl alias_ v_type=G type=ud num_elts=128 align=GRF alias=<%0, 0>\n"
                  _MOV8
                "}\n"
                 : "=rw"(target));
  }
};

template <typename T> struct SetZero<T, 16> {
  static inline void run(T &target) {
    asm volatile("{\n"
                  ".decl alias_ v_type=G type=ud num_elts=256 align=GRF alias=<%0, 0>\n"
                  _MOV16
                  "}\n"
                 : "=rw"(target));
  }
};

template <typename T> struct SetZero<T, 32> {
  static inline void run(T &target) {
    asm volatile("{\n"
                  ".decl alias_ v_type=G type=ud num_elts=512 align=GRF alias=<%0, 0>\n"
                  _MOV32
                  "}\n"
                 : "=rw"(target));
  }
};

#endif
