//
// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: MIT
//
#pragma once

struct barrier_test {
  barrier_test()
  {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto n_sg = item.get_sub_group().get_group_range()[0];
    auto sg_id = item.get_sub_group().get_group_id()[0];

    constexpr int n_barrier = 4;

    auto n_threads = n_sg / 4;
    auto id = sg_id % 4;

    named_barrier_init<4>();

    nbarrier_signal(id, n_threads);
    nbarrier_wait(id);
#endif
  }
};

struct barrier_test1 {
  barrier_test1()
  {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto n_sg = item.get_sub_group().get_group_range()[0];
    auto sg_id = item.get_sub_group().get_group_id()[0];

    constexpr int n_barrier = 4;

    uint8_t n_threads = n_sg / 4;
    uint8_t id = sg_id % 4;

    named_barrier_init<4>();

    BarrierPayload barrier(id,
        BarrierType::ProducerConsumer,
        n_threads, n_threads);

    nbarrier_signal(barrier);
    nbarrier_wait(id);
#endif
  }
};



