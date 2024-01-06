#pragma once

#include <sycl/sycl.hpp>
#include <iostream>

template <int ndev, int nsub>
sycl::device getSubDevice() {
  static auto devs = sycl::device::get_devices(sycl::info::device_type::gpu);
  auto dev = devs[ndev];
  try {
    static auto subs = dev.template create_sub_devices<
      sycl::info::partition_property::partition_by_affinity_domain>(
          sycl::info::partition_affinity_domain::numa);

    // swap sub-device 2 and 3 for reverting xelink cross connection
    int map_nsub = nsub;
    if (ndev == 1)
      map_nsub = nsub ^ 1;

    return subs[map_nsub];
  } catch (sycl::exception &e) {
    std::cout<<e.what()<<std::endl;
    return dev;
  };
}

template <int ndev, int nsub>
sycl::queue getQueue() {
  static sycl::queue q(
      getSubDevice<ndev, nsub>(),
      sycl::property_list {
        sycl::property::queue::enable_profiling(),
        sycl::property::queue::in_order()
      });
  return q;
}

sycl::queue currentQueue(int ndev, int nsub);
sycl::device currentSubDevice(int ndev, int nsub);

sycl::device currentSubDevice();
sycl::queue currentQueue();

template <typename F>
class release_guard {
  F f;
public:
  release_guard(F f) : f(f) {}
  ~release_guard() { f(); }
};

// Copy from sycl runtime, change the interface a little bit
template <typename T, typename Group, typename... Args>
std::enable_if_t<std::is_trivially_destructible<T>::value &&
                     sycl::detail::is_group<Group>::value,
                 sycl::local_ptr<typename std::remove_extent<T>::type>>
    __SYCL_ALWAYS_INLINE __shared__(Group g, Args &&...args) {
  (void)g;
#ifdef __SYCL_DEVICE_ONLY__
  __attribute__((opencl_local)) std::uint8_t *AllocatedMem =
      __sycl_allocateLocalMemory(sizeof(T), alignof(T));

  if constexpr (!std::is_trivial_v<T>) {
    sycl::id<3> Id = __spirv::initLocalInvocationId<3, sycl::id<3>>();
    if (Id == sycl::id<3>(0, 0, 0))
      new (AllocatedMem) T(std::forward<Args>(args)...);
    sycl::detail::workGroupBarrier();
  }
  return reinterpret_cast<
      __attribute__((opencl_local)) typename std::remove_extent<T>::type *>(AllocatedMem);
#else
  // Silence unused variable warning
  [&args...] {}();
  throw sycl::exception(sycl::errc::feature_not_supported,
      "sycl_ext_oneapi_local_memory extension is not supported on host device");
#endif
}

#define DIVUP(x, y) \
    (((x)+(y)-1)/(y))

#define ROUNDUP(x, y) \
    (DIVUP((x), (y))*(y))

#define ALIGN_POWER(x, y) \
    ((x) > (y) ? ROUNDUP(x, y) : ((y)/((y)/(x))))

#define ALIGN_SIZE(size, align) \
  size = ((size + (align) - 1) / (align)) * (align);
