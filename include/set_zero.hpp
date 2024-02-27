#pragma once

template <typename T, int N> struct SetZero;

#if defined(__SYCL_DEVICE_ONLY__)    

#define CONCAT(A, B) _CONCAT(A, B)
#define _CONCAT(A, B) A##B

#define _MOV1 mov (M1, 16) P1(0, 0)<1> 0x0:ud\n
#define _MOV2 _MOV1 mov (M1, 16) P1(1, 0)<1> 0x0:ud\n
#define _MOV4                                                                  \
  _MOV2 mov (M1, 16) P1(2, 0)<1> 0x0:ud\n                                 \
        mov (M1, 16) P1(3, 0)<1> 0x0:ud\n
#define _MOV8                                                                  \
  _MOV4 mov (M1, 16) P1(4, 0)<1> 0x0:ud\n                                 \
        mov (M1, 16) P1(5, 0)<1> 0x0:ud\n                                 \
        mov (M1, 16) P1(6, 0)<1> 0x0:ud\n                                 \
        mov (M1, 16) P1(7, 0)<1> 0x0:ud\n
#define _MOV16                                                                 \
  _MOV8 mov (M1, 16) P1(8, 0)<1> 0x0:ud\n                                 \
        mov (M1, 16) P1(9, 0)<1> 0x0:ud\n                                 \
        mov (M1, 16) P1(10, 0)<1> 0x0:ud\n                                \
        mov (M1, 16) P1(11, 0)<1> 0x0:ud\n                                \
        mov (M1, 16) P1(12, 0)<1> 0x0:ud\n                                \
        mov (M1, 16) P1(13, 0)<1> 0x0:ud\n                                \
        mov (M1, 16) P1(14, 0)<1> 0x0:ud\n                                \
        mov (M1, 16) P1(15, 0)<1> 0x0:ud\n

#define Stringify(A) _Stringify(A)
#define _Stringify(A) #A
#define MOV(num) Stringify(CONCAT(_MOV, num))



template <typename T> struct SetZero<T, 4> {
  static inline void run(T &target) {
    asm volatile("{\n"
                  ".decl P1 v_type=G type=ud num_elts=64 align=GRF alias=<%0, 0>\n" 
                  MOV(4)
                  "}\n"
                 : "=rw"(target));
  }
};

template <typename T> struct SetZero<T, 8> {
  static inline void run(T &target) {
    asm volatile("{\n"
                ".decl P1 v_type=G type=ud num_elts=128 align=GRF alias=<%0, 0>\n" 
                  MOV(8)
                "}\n"
                 : "=rw"(target));
  }
};

template <typename T> struct SetZero<T, 16> {
  static inline void run(T &target) {
    asm volatile("{\n"
                  ".decl P1 v_type=G type=ud num_elts=256 align=GRF alias=<%0, 0>\n" 
                  MOV(16)
                  "}\n"
                 : "=rw"(target));
  }
};

#endif