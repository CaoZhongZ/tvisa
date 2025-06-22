#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "gen_visa_templates.hpp"

template <int SG_SZ>
struct dump_sr {
  dump_sr(void* sink):sink(reinterpret_cast<uint32_t (*)[4]>(sink)) {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto lid = item.get_local_id()[0];
    auto group_id =  item.get_group().get_group_id()[0];
    if (lid < 4)
      sink[group_id][lid] = dumpSR();
#endif
  }

  uint32_t (* sink)[4];
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("DumpSR", "Dump launching mapping");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("g,group", "Number of group",
     cxxopts::value<size_t>()->default_value("2"))
    ("s,subgroup", "Number of subgroup",
     cxxopts::value<size_t>()->default_value("16"))
    ("w,sg_sz", "Subgroup size",
     cxxopts::value<size_t>()->default_value("16"))
    ;

  auto parsed_opts = opts.parse(argc, argv);
  auto n_group = parsed_opts["group"].as<size_t>();
  auto n_subgroup = parsed_opts["subgroup"].as<size_t>();
  auto sg_sz = parsed_opts["sg_sz"].as<size_t>();

  auto queue = currentQueue(0, 0);

  std::cout<<"Launch kernel("
    <<n_group * n_subgroup * 16<<", "
    <<n_subgroup * 16<<")"<<std::endl;

  auto sink_sz = n_group * sizeof(uint32_t) * 4;
  auto sink = (char *)sycl::malloc_device(sink_sz, queue);
  auto h_sink = (char *)sycl::malloc_host(sink_sz, queue);

  if (sg_sz == 16) {
    queue.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1> {
            sycl::range<1> {n_group * n_subgroup * sg_sz},
            sycl::range<1> {n_subgroup * sg_sz}}, dump_sr<16>(sink));
    });
  } else if (sg_sz == 32) {
    queue.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1> {
            sycl::range<1> {n_group * n_subgroup * sg_sz},
            sycl::range<1> {n_subgroup * sg_sz}}, dump_sr<32>(sink));
    });
  }

  queue.copy(h_sink, sink, sink_sz);
  queue.wait();
}
