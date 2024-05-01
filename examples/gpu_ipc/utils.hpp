#pragma once

#include <cstddef>

template <typename F>
class __scope_guard {
  F f;
  bool todo {true};
public:
  __scope_guard(const F &f) : f(f) {}
  void release() { f(); todo=false; }
  ~__scope_guard() { if (todo) release(); }
};

template <typename T>
void fill_pattern(T *input, int rank, size_t n) {
  for (int i = 0; i < n; ++ i)
    input[i] = (T)((i % 32) * (rank + 1));
}
