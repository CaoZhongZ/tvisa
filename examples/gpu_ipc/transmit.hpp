#pragma once

#include <sycl/sycl.hpp>
#include <gen_visa_templates.hpp>

#define divUp(x, m)  \
  ((x + m -1) / m)

#define alignUp(x, c) \
  (divUp(x, c) * c)

#include "rt64.hpp"
#include "rt64_128.hpp"

#include "parallel_transmit.hpp"
// #include "simple_transmit.hpp"
#include "bisect_transmit.hpp"
