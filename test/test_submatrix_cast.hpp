#pragma once

#include <iostream>
#include <sycl/sycl.hpp>

#include "sycl_misc.hpp"

template <typename T, int BH, int BW, int SG_SZ>
struct tileSplitAdd {
  tileSplitAdd(T* dst, const T* src,
      int surfaceH, int surfaceW, int surfaceP
  ): src(src), dst(dst), surfaceH(surfaceH), surfaceW(surfaceW), surfaceP(surfaceP)
  {}

  using tileT = __ArrayMatrix<T, BH, BW, DataShuffle::none, SG_SZ, 2>;

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (
      sycl::nd_item<2> pos
  ) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto group_id = pos.get_group().get_group_id();

    int x_off = group_id[1] * BW;
    int y_off = group_id[0] * BH;

    AddressPayload<BH, BW, 2> adrs(
        (void *)src, surfaceH, surfaceW, surfaceP, x_off, y_off
    );

    AddressPayload<BH, BW> adrs_0(
        (void *)src, surfaceH, surfaceW, surfaceP, x_off, y_off
    );
    AddressPayload<BH, BW> adrs_1(adrs_0);
    adrs_1.addSrc0AddrX(BW * sizeof(T));

    tileT tmp;

    tmp.load(adrs);

    auto tmp_0 = tmp.template subArrayView<0>();
    auto tmp_1 = tmp.template subArrayView<1>();

    tmp_0 = tmp_0 + tmp_1;
    tmp_1 = tmp_0;

    tmp_0.store(adrs_0);
    tmp_1.store(adrs_1);
#endif
  }

  const T* src;
  T* dst;
  int surfaceH, surfaceW, surfaceP;
};
