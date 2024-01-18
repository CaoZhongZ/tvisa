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
  barrier_test()
  {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto n_sg = item.get_sub_group().get_group_range()[0];
    auto sg_id = item.get_sub_group().get_group_id()[0];

    constexpr int n_barrier = 4;

    auto n_threads = n_sg / 4;
    auto id = sg_id % 4;

    nbarrier_signal(id, n_threads);
    nbarrier_wait(id);
#endif
  }
};

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

  std::cout<<"Launch kernel("
    <<n_group * n_subgroup * 16<<", "
    <<n_subgroup * 16<<")"<<std::endl;

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::nd_range<1> {
          sycl::range<1> {n_group * n_subgroup * 16},
          sycl::range<1> {n_subgroup * 16}}, barrier_test());
  });

  queue.wait();
}
