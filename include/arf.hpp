#pragma once

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
