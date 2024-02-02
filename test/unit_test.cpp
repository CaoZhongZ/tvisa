#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "gen_visa_templates.hpp"

size_t parse_nelems(const std::string& nelems_string) {
  size_t base = 1;
  size_t pos = nelems_string.rfind("K");
  if (pos != std::string::npos) {
    base = 1024ull;
  } else {
    pos = nelems_string.rfind("M");
    if (pos != std::string::npos)
      base = 1024 * 1024ull;
    else {
      pos = nelems_string.rfind("G");
      if (pos != std::string::npos)
        base = 1024 * 1024 * 1024ull;
    }
  }

  return stoull(nelems_string) * base;
}

template <typename T>
void fill_sequential(T *p, int rank, size_t nelems) {
  for (size_t i = 0; i < nelems; ++ i) {
    p[i] = i + rank;
  }
}

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("g,group", "Number of group",
     cxxopts::value<size_t>()->default_value("1"))
    ("s,subgroup", "Number of subgroup",
     cxxopts::value<size_t>()->default_value("16"))
    ("n,numbarrier", "Number of barriers",
     cxxopts::value<size_t>()->default_value("4"))
    ;

  auto parsed_opts = opts.parse(argc, argv);
  auto n_group = parsed_opts["group"].as<size_t>();
  auto n_subgroup = parsed_opts["subgroup"].as<size_t>();
  auto n_barrier = parsed_opts["numbarrier"].as<size_t>();

  auto queue = currentQueue(0, 0);
}
