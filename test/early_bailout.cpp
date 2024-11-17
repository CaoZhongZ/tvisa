#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "gen_visa_templates.hpp"

#define SG_SZ 16

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

struct barrier_test {
  barrier_test(size_t bound) : bound(bound)
  {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
    auto id = item.get_global_id(0);

    if (id >= bound)
      return;

    auto n_threads = item.get_sub_group().get_group_range()[0];
    
#if defined(__SYCL_DEVICE_ONLY__)
    atob_barrier(n_threads);
#else
    item.barrier(sycl::access::fence_space::local_space);
#endif
  }

  size_t bound;
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("g,group", "Number of group",
     cxxopts::value<size_t>()->default_value("1"))
    ("s,subgroup", "Number of subgroup",
     cxxopts::value<size_t>()->default_value("16"))
    ("b,bound", "Boundary from where everybody bailout",
     cxxopts::value<size_t>()->default_value("4"))
    ;

  auto parsed_opts = opts.parse(argc, argv);
  auto n_group = parsed_opts["group"].as<size_t>();
  auto n_subgroup = parsed_opts["subgroup"].as<size_t>();
  auto n_bound = parsed_opts["bound"].as<size_t>();

  auto queue = currentQueue(0, 0);

  std::cout<<"Launch kernel("
    <<n_group * n_subgroup * 16<<", "
    <<n_subgroup * 16<<")"<<std::endl;

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::nd_range<1> {
          sycl::range<1> {n_group * n_subgroup * 16},
          sycl::range<1> {n_subgroup * 16}}, barrier_test(n_bound));
  });

  queue.wait();
}
