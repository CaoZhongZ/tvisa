#pragma once

#include <sycl/sycl.hpp>

// barrier, named barrier and split barrier
void barrier() {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
  asm volatile ("barrier");
#else
  static_assert(false,
    "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
}

void nbarrier_wait(uint8_t id) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
  asm volatile ("nbarrier.wait %0" :: "rw"(id));
#else
  static_assert(false,
    "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
}

void nbarrier_signal(uint8_t n_threads, uint8_t id) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
  asm volatile ("nbarrier.signal %0 %1" :: "rw"(n_threads), "rw"(id));
#else
  static_assert(false,
    "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
}

void sbarrier_wait() {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
  asm volatile ("nbarrier.wait);
#else
  static_assert(false,
    "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
}

void sbarrier_signal() {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
  asm volatile ("nbarrier.signal");
#else
  static_assert(false,
    "Not supported on host, wrap your code with __SYCL_DEVICE_ONLY__");
#endif
}

enum LscType {
  Ugm = 0, Ugml, Tgm, Slm
};

enum LscFenceOp {
  None = 0, Evict, Invalidate, Discard, Clean, FlushL3
};

enum LscScope {
  Group = 0, Local, Tile, GPU, GPUs, SystemRel, SystemAcq
};

template <LscType Id, LscFenceOp Op, LscScope Scope>
void lscFence();

template void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Group>() {
  asm("lsc_fence.ugm.none.group");
}

template void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Local>() {
  asm("lsc_fence.ugm.none.local");
}

template void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Tile>() {
  asm("lsc_fence.ugm.none.tile");
}

template void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::GPU>() {
  asm("lsc_fence.ugm.none.gpu");
}

template void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::GPUs>() {
  asm("lsc_fence.ugm.none.gpus");
}

template void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::SystemRel>() {
  asm("lsc_fence.ugm.none.system");
}

template void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::SystemAcq>() {
  asm("lsc_fence.ugm.none.sysacq");
}

template void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Group>() {
  asm("lsc_fence.ugm.evict.group");
}

template void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Local>() {
  asm("lsc_fence.ugm.evict.local");
}

template void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Tile>() {
  asm("lsc_fence.ugm.evict.tile");
}

template void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::GPU>() {
  asm("lsc_fence.ugm.evict.gpu");
}

template void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::GPUs>() {
  asm("lsc_fence.ugm.evict.gpus");
}

template void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::SystemRel>() {
  asm("lsc_fence.ugm.evict.system");
}

template void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::SystemAcq>() {
  asm("lsc_fence.ugm.evict.sysacq");
}

template void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Group>() {
  asm("lsc_fence.ugm.clean.group");
}

template void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Local>() {
  asm("lsc_fence.ugm.clean.local");
}

template void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Tile>() {
  asm("lsc_fence.ugm.clean.tile");
}

template void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::GPU>() {
  asm("lsc_fence.ugm.clean.gpu");
}

template void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::GPUs>() {
  asm("lsc_fence.ugm.clean.gpus");
}

template void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::SystemRel>() {
  asm("lsc_fence.ugm.clean.system");
}

template void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::SystemAcq>() {
  asm("lsc_fence.ugm.clean.sysacq");
}
