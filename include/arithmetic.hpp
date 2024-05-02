//
// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: MIT
//
#pragma once

#include <sycl/sycl.hpp>

//
// These arithmetics are for doing math on reinterpreted registers
// But with original prediate control
//
template <typename T, int SubGroupSize = 16>
struct Add {
  template <typename D> static inline D run (D src0, D src1);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
template <int P> struct Add<sycl::half, 16> {
  template <> static inline sycl::vec<uint32_t, 2> run (
      const sycl::vec<uint32_t, 2> src0, const sycl::vec<uint32_t, 2> src1
  ) {
    uint32_t ret;
    asm volatile ("{\n"
        ".decl aliasA_0 v_type=G type=hf num_elts=32 align=GRF alias=<%1, 0>\n"
        ".decl aliasA_1 v_type=G type=hf num_elts=32 align=GRF alias=<%1, 64>\n"
        ".decl aliasB_0 v_type=G type=hf num_elts=32 align=GRF alias=<%2, 0>\n"
        ".decl aliasB_1 v_type=G type=hf num_elts=32 align=GRF alias=<%2, 64>\n"
        ".decl aliasC_0 v_type=G type=hf num_elts=32 align=GRF alias=<%0, 0>\n"
        ".decl aliasC_1 v_type=G type=hf num_elts=32 align=GRF alias=<%0, 64>\n"
        "add (M1, 32) aliasC_0(0,0)<1> aliasA_0(0,0)<1;1,0> aliasB_0(0,0)<1;1,0>\n"
        "add (M1, 32) aliasC_1(0,0)<1> aliasA_1(0,0)<1;1,0> aliasB_1(0,0)<1;1,0>\n"
        "}\n"
        : "=rw"(ret) : "rw"(src0), "rw"(src1));
    return ret;
  }
};

#endif

//
// Usage: auto c = addAs<sycl::half, 16>(a, b);
// XXX: might be a little confusing while 16 is meant for subgroup
//
template <typename Interp, int SubGroupSize=16, typename Origin>
Origin addAs(const Origin src0, const Origin src1) {
  return Add<Interp, SubGroupSize>::run(src0, src1);
}
