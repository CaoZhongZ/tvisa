#pragma once

#include <sycl/sycl.hpp>

// barrier, named barrier and split barrier
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

extern SYCL_EXTERNAL void named_barrier_init(uint8_t count);

static inline void barrier() {
  asm volatile ("barrier");
}

static inline void nbarrier_wait(uint8_t id) {
  asm volatile ("nbarrier.wait %0(0,0)<0;1,0>" :: "rw"(id));
}

static inline void nbarrier_signal(uint8_t id, uint8_t n_threads) {
  asm volatile ("nbarrier.signal %0(0,0)<0;1,0> %1(0,0)<0;1,0>" :: "rw"(id), "rw"(n_threads));
}

static inline void sbarrier_wait() {
  asm volatile ("nbarrier.wait");
}

static inline void sbarrier_signal() {
  asm volatile ("nbarrier.signal");
}
#endif

enum LscType {
  Ugm = 0, Ugml, Tgm, Slm
};

enum LscFenceOp {
  None = 0, Evict, Invalidate, Discard, Clean, FlushL3
};

enum LscScope {
  Group = 0, Local, Tile, GPU, GPUs, SystemRel, SystemAcq
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

template <LscType Id, LscFenceOp Op, LscScope Scope>
void lscFence();

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Group>() {
  asm("lsc_fence.ugm.none.group");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Local>() {
  asm("lsc_fence.ugm.none.local");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Tile>() {
  asm("lsc_fence.ugm.none.tile");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::GPU>() {
  asm("lsc_fence.ugm.none.gpu");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::GPUs>() {
  asm("lsc_fence.ugm.none.gpus");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::SystemRel>() {
  asm("lsc_fence.ugm.none.system");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::SystemAcq>() {
  asm("lsc_fence.ugm.none.sysacq");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Group>() {
  asm("lsc_fence.ugm.evict.group");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Local>() {
  asm("lsc_fence.ugm.evict.local");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Tile>() {
  asm("lsc_fence.ugm.evict.tile");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::GPU>() {
  asm("lsc_fence.ugm.evict.gpu");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::GPUs>() {
  asm("lsc_fence.ugm.evict.gpus");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::SystemRel>() {
  asm("lsc_fence.ugm.evict.system");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::SystemAcq>() {
  asm("lsc_fence.ugm.evict.sysacq");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Group>() {
  asm("lsc_fence.ugm.clean.group");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Local>() {
  asm("lsc_fence.ugm.clean.local");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Tile>() {
  asm("lsc_fence.ugm.clean.tile");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::GPU>() {
  asm("lsc_fence.ugm.clean.gpu");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::GPUs>() {
  asm("lsc_fence.ugm.clean.gpus");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::SystemRel>() {
  asm("lsc_fence.ugm.clean.system");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::SystemAcq>() {
  asm("lsc_fence.ugm.clean.sysacq");
}

#endif
