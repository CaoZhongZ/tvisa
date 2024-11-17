#pragma once

#include <sycl/sycl.hpp>

// barrier, named barrier and split barrier
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

//
// Use OpenCL named barrier initialization to trigger compiler
// allocate named barrier.
//
extern SYCL_EXTERNAL void named_barrier_init(int count);

template <int N> void named_barrier_init() {
  if constexpr (N -1 > 0)
    named_barrier_init<N -1>();

  named_barrier_init(N);
}

static inline void barrier() {
  asm volatile ("barrier\n");
}

static inline void nbarrier_wait(uint8_t id) {
  asm volatile ("nbarrier.wait %0(0,0)<0;1,0>\n" :: "rw"(id));
}

static inline void nbarrier_signal(uint8_t id, uint8_t n_threads) {
  asm volatile (
      "nbarrier.signal %0(0,0)<0;1,0> %1(0,0)<0;1,0>\n"
      :: "rw"(id), "rw"(n_threads));
}

// raw send version with full named barrier potential exposed
static inline void nbarrier_signal(const BarrierPayload& Barrier) {
  asm volatile (
      "raw_sends.3.1.0.0 (M1, 1) 0x0:ud 0x02000004:ud %0.0 V0.0 V0.0\n"
      :: "rw"(Barrier.getPayload()));
}

static inline void atob_barrier(int n_threads) {
  BarrierPayload barrier (0, true, 0, n_threads, n_threads);
  asm volatile (
      "raw_sends.3.1.0.0 (M1, 1) 0x0:ud 0x02000804:ud %0.0 V0.0 V0.0\n"
      :: "rw"(barrier.getPayload()));
}

static inline void sbarrier_wait() {
  asm volatile ("sbarrier.wait\n");
}

static inline void sbarrier_signal() {
  asm volatile ("sbarrier.signal\n");
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

void swFence() {
  asm volatile ("fence_sw\n");
}

template <LscType Id, LscFenceOp Op, LscScope Scope>
void lscFence();

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Group>() {
  asm volatile ("lsc_fence.ugm.none.group\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Local>() {
  asm volatile ("lsc_fence.ugm.none.local\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::Tile>() {
  asm volatile ("lsc_fence.ugm.none.tile\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::GPU>() {
  asm volatile ("lsc_fence.ugm.none.gpu\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::GPUs>() {
  asm volatile ("lsc_fence.ugm.none.gpus\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::SystemRel>() {
  asm volatile ("lsc_fence.ugm.none.system\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::None, LscScope::SystemAcq>() {
  asm volatile ("lsc_fence.ugm.none.sysacq\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Group>() {
  asm volatile ("lsc_fence.ugm.evict.group\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Local>() {
  asm volatile ("lsc_fence.ugm.evict.local\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::Tile>() {
  asm volatile ("lsc_fence.ugm.evict.tile\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::GPU>() {
  asm volatile ("lsc_fence.ugm.evict.gpu\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::GPUs>() {
  asm volatile ("lsc_fence.ugm.evict.gpus\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::SystemRel>() {
  asm volatile ("lsc_fence.ugm.evict.system\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Evict, LscScope::SystemAcq>() {
  asm volatile ("lsc_fence.ugm.evict.sysacq\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Group>() {
  asm volatile ("lsc_fence.ugm.clean.group\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Local>() {
  asm volatile ("lsc_fence.ugm.clean.local\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::Tile>() {
  asm volatile ("lsc_fence.ugm.clean.tile\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::GPU>() {
  asm volatile ("lsc_fence.ugm.clean.gpu\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::GPUs>() {
  asm volatile ("lsc_fence.ugm.clean.gpus\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::SystemRel>() {
  asm volatile ("lsc_fence.ugm.clean.system\n");
}

template <> void lscFence<LscType::Ugm, LscFenceOp::Clean, LscScope::SystemAcq>() {
  asm volatile ("lsc_fence.ugm.clean.sysacq\n");
}

#endif
