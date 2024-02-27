#pragma once

#include <cstdint>

struct MMA8x32x16 {
  static constexpr uint32_t mma_m_in_elem = 8;
  static constexpr uint32_t mma_n_in_elem = 16;
  static constexpr uint32_t mma_k_in_bytes = 32;
};


struct MMA32x32x16 {
  static constexpr uint32_t mma_m_in_elem = 32;
  static constexpr uint32_t mma_n_in_elem = 16;
  static constexpr uint32_t mma_k_in_bytes = 32;
};

struct MMA32x32x64 {
  static constexpr uint32_t mma_m_in_elem = 32;
  static constexpr uint32_t mma_n_in_elem = 64;
  static constexpr uint32_t mma_k_in_bytes = 32;
};