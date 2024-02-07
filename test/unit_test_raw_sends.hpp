#pragma once

#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "gen_visa_templates.hpp"

static inline sycl::id<2> get_subgroup_no(sycl::nd_item<2> pos) {
  auto group_id = pos.get_group().get_group_id();
  auto subgroup_id = pos.get_sub_group().get_group_id();
  auto subgroup_sz = pos.get_sub_group().get_group_range();

  return group_id * subgroup_sz + subgroup_id;
}

static inline sycl::id<2> get_subgroup_linear_no(sycl::nd_item<2> pos) {
  auto group_id = pos.get_group().get_linear_group_id();
  auto subgroup_id = pos.get_sub_group().get_linear_group_id();
  auto subgroup_sz = pos.get_sub_group().get_linear_group_range();

  return group_id * subgroup_sz + subgroup_id;
}

//
// Each canvas dimension shouldn't beyond 16MB
//
template <typename T, int BH, int BW, int SG_SZ>
struct test_tile_load {
  test_tile_load(
      T* dst, const T* src,
      int surfaceH, int surfaceW, int surfaceP
  ) : src(src), dst(dst),
    surfaceH(surfaceH), surfaceW(surfaceW),
    surfaceP(surfaceP)
  {}

  using matrix = __Matrix<T, BH, BW, DataShuffle::none, SG_SZ>;

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (
      sycl::nd_item<2> pos
  ) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto subgroup_id = get_sub_group_no(pos);

    int x_off = subgroup_id[1] * BW;
    int y_off = subgroup_id[0] * BH;

    AddressPayload<BH, BW> adrs(
      src, surfaceH, surfaceW, surfaceP, x_off, y_off
    );
    matrix tmp;
    tmp.zero();
    tmp.load(adrs);

    auto subgroup_linear_id = get_subgroup_linear_no(pos);

    auto dump_off =
      subgroup_linear_id * sizeof(matrix)/sizeof(T)
      + pos.get_sub_group().get_linear_local_id();

    tmp.dump(dst + dump_off);
#endif
  }

private:
  T* dst;
  const T* src;
  int surfaceH;
  int surfaceW;
  int surfaceP;
};

template <typename T, int BH, int BW, int SG_SZ>
struct test_tile_store {
  test_tile_load(
      T* dst, const T* src,
      int surfaceH, int surfaceW, int surfaceP
  ) : src(src), dst(dst),
    surfaceH(surfaceH), surfaceW(surfaceW),
    surfaceP(surfaceP)
  {}

  using matrix = __Matrix<T, BH, BW, DataShuffle::none, SG_SZ>;

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (
      sycl::nd_item<2> pos
  ) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto subgroup_id = get_sub_group_no(pos);

    int x_off = subgroup_id[1] * BW;
    int y_off = subgroup_id[0] * BH;

    AddressPayload<BW, BH> adrs(
      dst, surfaceH, surfaceW, surfaceP, x_off, y_off
    );
    matrix tmp;

    auto subgroup_linear_id = get_subgroup_linear_no(pos);
    auto dump_off =
      subgroup_linear_id * sizeof(matrix)/sizeof(T)
      + pos.get_sub_group().get_linear_local_id();

    lscLoad<SG_SZ>(tmp.getStorage(), src + dump_off);

    tmp.store(adrs);
#endif
  }

private:
  T* dst;
  const T* src;
  int surfaceW;
  int surfaceH;
  int surfaceP;
};

//
// Ideally test range should be
// type: int8_t, sycl::half, float, uint64_t
// Block height: 1 - 32
// Block width: 1 - 16 lane, interpreted by data-type
// Sub-group SZ: 16, 32
//
