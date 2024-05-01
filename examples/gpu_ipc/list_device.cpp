#include <iostream>
#include <sycl/sycl.hpp>

int main(int argc, char *argv[]) {
  auto devs = sycl::device::get_devices(sycl::info::device_type::gpu);
  for (auto& dev : devs) {
    auto subs = dev.template create_sub_devices<
      sycl::info::partition_property::partition_by_affinity_domain>(
          sycl::info::partition_affinity_domain::numa);
    for (auto& sub : subs) {
      std::cout<<sub.get_info<sycl::info::device::name>()<<std::endl;
    }
  }
}
