//
// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: MIT
//
#pragma once

#include <sycl/sycl.hpp>
#include <iostream>

template <int ndev, int nsub>
sycl::device getSubDevice() {
  static auto devs = sycl::device::get_devices(sycl::info::device_type::gpu);
  auto dev = devs[ndev];
  try {
    static auto subs = dev.template create_sub_devices<
      sycl::info::partition_property::partition_by_affinity_domain>(
          sycl::info::partition_affinity_domain::numa);

    // swap sub-device 2 and 3 for reverting xelink cross connection
    int map_nsub = nsub;
    if (ndev == 1)
      map_nsub = nsub ^ 1;

    return subs[map_nsub];
  } catch (sycl::exception &e) {
    return dev;
  };
}

template <int ndev, int nsub>
sycl::queue getQueue() {
  static sycl::queue q(
      getSubDevice<ndev, nsub>(),
      sycl::property_list {
        sycl::property::queue::enable_profiling(),
        sycl::property::queue::in_order()
      });
  return q;
}

sycl::queue currentQueue(int ndev, int nsub);
sycl::device currentSubDevice(int ndev, int nsub);

sycl::device currentSubDevice();
sycl::queue currentQueue();

#define DIVUP(x, y) \
    (((x)+(y)-1)/(y))

#define ROUNDUP(x, y) \
    (DIVUP((x), (y))*(y))

#define ALIGN_POWER(x, y) \
    ((x) > (y) ? ROUNDUP(x, y) : ((y)/((y)/(x))))

#define ALIGN_SIZE(size, align) \
  size = ((size + (align) - 1) / (align)) * (align);
