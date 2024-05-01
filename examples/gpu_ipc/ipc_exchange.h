#pragma once

#include <sycl/sycl.hpp>
#include <level_zero/ze_api.h>

void *mmap_host(size_t map_size, ze_ipc_mem_handle_t ipc_handle);

ze_ipc_mem_handle_t open_all_ipc_mems(
    sycl::queue queue, void *ptr, int rank, int world,
    void *peer_bases[], size_t offsets[], int instance = 0
);
