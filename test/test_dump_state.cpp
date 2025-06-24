#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "gen_visa_templates.hpp"

template <int SG_SZ>
struct dump_sr {
  dump_sr(void* sink):sink(sink) {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::nd_item<1> item) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto sub_group = item.get_sub_group();
    auto lid = sub_group.get_local_id()[0];
    auto sub_id = sub_group.get_group_id()[0];
    auto group_id =  item.get_group().get_group_id()[0];
    auto sr = dumpSR();

    auto sub_sz = sub_group.get_group_range()[0];
    auto _sink = reinterpret_cast<uint32_t (*)[sub_sz][4]>(sink);
    if (lid < 4) {
      _sink[group_id][sub_id][lid] = sr;
      // sycl::ext::oneapi::experimental::printf("%d@%d:%#x\n", lid, group_id, sr);
    }

    sycl::group_barrier(item.get_group());
#endif
  }

  void *sink;
};

union StateRegister {
  struct {
    unsigned tid:3;
    unsigned resv0:1;
    unsigned euid:3;
    unsigned resv1:1;
    unsigned subsliceid:2;
    unsigned resv2:1;
    unsigned sliceid:5;
    unsigned resv3:3;
    unsigned priority:4;
    unsigned p_class:1;
    unsigned ffid:4;
  } sr0_0;

  struct {
    unsigned ieeeexception:6;
    unsigned resv0:2;
    unsigned replaypass2:1;
    unsigned resv1:1;
    unsigned discard:1;
    unsigned resv2:10;
    unsigned fftid:10;
  } sr0_1;

  uint32_t raw;
};

void parseSR(int index, uint32_t sr_value) {
  StateRegister sr;
  sr.raw = sr_value;
  switch(index) {
  case 0:
    printf("tid:%d, euid:%d, subsliceid:%d, sliceid:%d\n", sr.sr0_0.tid, sr.sr0_0.euid, sr.sr0_0.subsliceid, sr.sr0_0.sliceid);
    break;
  }
}

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

  auto sink_sz = n_group * n_subgroup * sizeof(uint32_t) * 4;
  auto sink = sycl::malloc_device(sink_sz, queue);
  auto h_sink = sycl::malloc_host(sink_sz, queue);

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

  queue.memcpy(h_sink, sink, sink_sz);
  queue.wait();

  auto _h_sink = reinterpret_cast<uint32_t (*)[n_subgroup][4]>(h_sink);

  for (size_t i = 0; i < n_group; ++ i) {
    for (size_t j = 0; j < n_subgroup; ++ j) {
      printf("%lu,%lu:", i, j);
      parseSR(0, _h_sink[i][j][0]);
    }
  }
}
