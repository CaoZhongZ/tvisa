#pragma once

template <typename T, typename message_t, typename AccumT>
struct accumAdd {
  static inline AccumT run (AccumT acc, message_t a, message_t b);
};

template <typename message_t, typename AccumT>
struct accumAdd<sycl::half> {
  static inline AccumT run (AccumT acc, message_t a, message_t b) {
  }
}
