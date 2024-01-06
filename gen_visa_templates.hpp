#pragma once

#include <sycl/sycl.hpp>
#include "lsc.hpp"

enum DataShuffle {
  none, transpose, vnni
};

enum CacheCtrl {
  L1STATE_L3MOCS = 0,
  L1UC_L3UC,
  L1C_L3UC,
  L1C_L3C,
  L1S_L3UC,
};
