#pragma once

union sr0_0 {
  struct {
    unsigned tid:3;
    unsigned resv0:1;
    unsigned euid:3;
    unsigned resv1:1;
    unsigned subsliceid:2;
    unsigned resv2:1;
    unsigned sliceid:5;
    unsigned resv3:3;
    unsigned priority:4;
    unsigned p_class:1;
    unsigned ffid:4;
  };
  uint32_t raw;
};

union sr0_1 {
  struct {
    unsigned ieeeexception:6;
    unsigned resv0:2;
    unsigned replaypass2:1;
    unsigned resv1:1;
    unsigned discard:1;
    unsigned resv2:10;
    unsigned fftid:10;
  };
  uint32_t raw;
};

static inline uint32_t dumpSR() {
  uint32_t sr; // 4 lanes used
#if defined(__SYCL_DEVICE_ONLY__)
  asm volatile (
      "mov (M1, 1) %0(0, 0)<1> %%sr0(0, 0)<0;1,0>\n"
      "mov (M1, 1) %0(0, 1)<1> %%sr0(0, 1)<0;1,0>\n"
      "mov (M1, 1) %0(0, 2)<1> %%sr0(0, 2)<0;1,0>\n"
      "mov (M1, 1) %0(0, 3)<1> %%sr0(0, 3)<0;1,0>\n"
    : "=rw"(sr));
#else
  sr = 0;
#endif
  return sr;
}

static inline uint32_t dumpCE() {
  uint32_t ce;
#if defined(__SYCL_DEVICE_ONLY__)
  asm volatile (
      "mov (M1, 1) %0(0, 0)<1> %%ce0(0, 0)<0;1,0>\n"
    : "=rw"(ce));
#else
  ce = 0;
#endif
  return ce;
}
