#include <array>
#include <cstdint>
#include <iostream>
#include <sycl/sycl.hpp>

#include "gen_visa_templates.hpp"

constexpr int SG_SZ = 16;

#if defined(__SYCL_TARGET_INTEL_GPU_CRI__)
constexpr int POLICY_COUNT = 15;

static_assert(CacheCtrl::DEFAULT == 0);
static_assert(CacheCtrl::L1UC_L2UC_L3UC == 2);
static_assert(CacheCtrl::L1UC_L2UC_L3WB == 3);
static_assert(CacheCtrl::L1UC_L2WB_L3UC == 4);
static_assert(CacheCtrl::L1UC_L2WB_L3WB == 5);
static_assert(CacheCtrl::L1WT_L2UC_L3UC == 6);
static_assert(CacheCtrl::L1WT_L2UC_L3WB == 7);
static_assert(CacheCtrl::L1WT_L2WB_L3UC == 8);
static_assert(CacheCtrl::L1WT_L2WB_L3WB == 9);
static_assert(CacheCtrl::L1S_L2UC_L3UC == 10);
static_assert(CacheCtrl::L1S_L2UC_L3WB == 11);
static_assert(CacheCtrl::L1S_L2WB_L3UC == 12);
static_assert(CacheCtrl::L1WB_L2UC_L3UC == 13);
static_assert(CacheCtrl::L1WB_L2WB_L3UC == 14);
static_assert(CacheCtrl::L1WB_L2UC_L3WB == 15);
static_assert(CacheCtrl::L1UC_L3UC == CacheCtrl::L1UC_L2UC_L3UC);
static_assert(CacheCtrl::L1WB_L3WB == CacheCtrl::L1WB_L2WB_L3UC);
#else
constexpr int POLICY_COUNT = 8;

static_assert(CacheCtrl::DEFAULT == 0);
static_assert(CacheCtrl::L1UC_L3UC == 1);
static_assert(CacheCtrl::L1UC_L3C == 2);
static_assert(CacheCtrl::L1C_L3UC == 3);
static_assert(CacheCtrl::L1C_L3C == 4);
static_assert(CacheCtrl::L1S_L3UC == 5);
static_assert(CacheCtrl::L1S_L3C == 6);
static_assert(CacheCtrl::L1IAR_L3C == 7);
#endif

template <CacheCtrl LoadCtrl, CacheCtrl StoreCtrl>
static inline void compileRawCacheCtrl(const std::uint32_t *src) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
  // Keep every raw specialization in reachable device code so vISA parses it.
  // The test input never uses this sentinel, so these sends are not executed.
  if (src[0] == UINT32_MAX) {
    AddressPayload<1, 16> address(
        src, 1, 16 * sizeof(std::uint32_t),
        16 * sizeof(std::uint32_t), 0, 0);
    __ArrayMatrix<
        std::uint32_t, 1, 16, DataShuffle::none, SG_SZ, 1
    > data;
    lscLoad<LoadCtrl>(data, address);
    lscStore<StoreCtrl>(address, data);
#if defined(__SYCL_TARGET_INTEL_GPU_CRI__)
    if constexpr (LoadCtrl != CacheCtrl::L1UC_L2UC_L3UC)
#else
    if constexpr (LoadCtrl != CacheCtrl::L1UC_L3UC)
#endif
      RawPrefetch<2, 0, DataShuffle::none, LoadCtrl>::run(address);

    if constexpr (LoadCtrl == CacheCtrl::DEFAULT) {
      RawSendLoad<
          2, 1, DataShuffle::transpose, LoadCtrl
      >::run(data.getStorage(), address);
      RawSendLoad<
          1, 1, DataShuffle::vnni, LoadCtrl
      >::run(data.getStorage(), address);
      RawPrefetch<
          2, 0, DataShuffle::transpose, LoadCtrl
      >::run(address);
      RawPrefetch<
          1, 0, DataShuffle::vnni, LoadCtrl
      >::run(address);
    }

    AddressPayload<32, 16> largeAddress(
        src, 32, 16 * sizeof(std::uint16_t),
        16 * sizeof(std::uint16_t), 0, 0);
    __ArrayMatrix<
        std::uint16_t, 32, 16, DataShuffle::none, SG_SZ, 1
    > largeData;
    RawSendStore32_32<
        1, 16, DataShuffle::none, StoreCtrl
    >::run(largeAddress, largeData.getStorage());
  }
#endif
}

template <CacheCtrl LoadCtrl, CacheCtrl StoreCtrl>
static inline void copyWithCacheCtrl(
    std::uint32_t *dst, const std::uint32_t *src, std::size_t index) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
  std::uint32_t value;
  lscPrefetch<std::uint32_t, 1, SG_SZ, LoadCtrl>(
      const_cast<std::uint32_t *>(src + index));
  lscLoad<SG_SZ, LoadCtrl>(
      value, const_cast<std::uint32_t *>(src + index));
  lscStore<SG_SZ, StoreCtrl>(dst + index, value);
  compileRawCacheCtrl<LoadCtrl, StoreCtrl>(src);
#else
  dst[index] = src[index];
#endif
}

class TestCriCacheCtrl;

int main() {
  sycl::queue queue;
  constexpr std::size_t elementCount = POLICY_COUNT * SG_SZ;

  std::array<std::uint32_t, elementCount> input;
  std::array<std::uint32_t, elementCount> output = {};
  for (std::size_t i = 0; i < elementCount; ++i)
    input[i] = static_cast<std::uint32_t>(i + 1);

  auto *src = sycl::malloc_device<std::uint32_t>(elementCount, queue);
  auto *dst = sycl::malloc_device<std::uint32_t>(elementCount, queue);
  queue.copy(input.data(), src, elementCount).wait();

  queue.submit([&](sycl::handler &handler) {
    handler.parallel_for<TestCriCacheCtrl>(
        sycl::nd_range<1>(elementCount, SG_SZ),
        [=](sycl::nd_item<1> item)
            [[sycl::reqd_sub_group_size(SG_SZ)]] {
          auto index = item.get_global_linear_id();
          switch (item.get_group_linear_id()) {
#if defined(__SYCL_TARGET_INTEL_GPU_CRI__)
          case 0:
            copyWithCacheCtrl<CacheCtrl::DEFAULT, CacheCtrl::DEFAULT>(
                dst, src, index);
            break;
          case 1:
            copyWithCacheCtrl<CacheCtrl::L1UC_L2UC_L3UC,
                              CacheCtrl::L1UC_L2UC_L3UC>(dst, src, index);
            break;
          case 2:
            copyWithCacheCtrl<CacheCtrl::L1UC_L2UC_L3C,
                              CacheCtrl::L1UC_L2UC_L3WB>(dst, src, index);
            break;
          case 3:
            copyWithCacheCtrl<CacheCtrl::L1UC_L2C_L3UC,
                              CacheCtrl::L1UC_L2WB_L3UC>(dst, src, index);
            break;
          case 4:
            copyWithCacheCtrl<CacheCtrl::L1UC_L2C_L3C,
                              CacheCtrl::L1UC_L2WB_L3WB>(dst, src, index);
            break;
          case 5:
            copyWithCacheCtrl<CacheCtrl::L1C_L2UC_L3UC,
                              CacheCtrl::L1WT_L2UC_L3UC>(dst, src, index);
            break;
          case 6:
            copyWithCacheCtrl<CacheCtrl::L1C_L2UC_L3C,
                              CacheCtrl::L1WT_L2UC_L3WB>(dst, src, index);
            break;
          case 7:
            copyWithCacheCtrl<CacheCtrl::L1C_L2C_L3UC,
                              CacheCtrl::L1WT_L2WB_L3UC>(dst, src, index);
            break;
          case 8:
            copyWithCacheCtrl<CacheCtrl::L1C_L2C_L3C,
                              CacheCtrl::L1WT_L2WB_L3WB>(dst, src, index);
            break;
          case 9:
            copyWithCacheCtrl<CacheCtrl::L1S_L2UC_L3UC,
                              CacheCtrl::L1S_L2UC_L3UC>(dst, src, index);
            break;
          case 10:
            copyWithCacheCtrl<CacheCtrl::L1S_L2UC_L3C,
                              CacheCtrl::L1S_L2UC_L3WB>(dst, src, index);
            break;
          case 11:
            copyWithCacheCtrl<CacheCtrl::L1S_L2C_L3UC,
                              CacheCtrl::L1S_L2WB_L3UC>(dst, src, index);
            break;
          case 12:
            copyWithCacheCtrl<CacheCtrl::L1S_L2C_L3C,
                              CacheCtrl::L1WB_L2UC_L3UC>(dst, src, index);
            break;
          case 13:
            copyWithCacheCtrl<CacheCtrl::L1RI_L2RI_L3RI,
                              CacheCtrl::L1WB_L2WB_L3UC>(dst, src, index);
            break;
          case 14:
            copyWithCacheCtrl<CacheCtrl::DEFAULT,
                              CacheCtrl::L1WB_L2UC_L3WB>(dst, src, index);
            break;
#else
          case 0:
            copyWithCacheCtrl<CacheCtrl::DEFAULT, CacheCtrl::DEFAULT>(
                dst, src, index);
            break;
          case 1:
            copyWithCacheCtrl<CacheCtrl::L1UC_L3UC,
                              CacheCtrl::L1UC_L3UC>(dst, src, index);
            break;
          case 2:
            copyWithCacheCtrl<CacheCtrl::L1UC_L3C,
                              CacheCtrl::L1UC_L3WB>(dst, src, index);
            break;
          case 3:
            copyWithCacheCtrl<CacheCtrl::L1C_L3UC,
                              CacheCtrl::L1WT_L3UC>(dst, src, index);
            break;
          case 4:
            copyWithCacheCtrl<CacheCtrl::L1C_L3C,
                              CacheCtrl::L1WT_L3WB>(dst, src, index);
            break;
          case 5:
            copyWithCacheCtrl<CacheCtrl::L1S_L3UC,
                              CacheCtrl::L1S_L3UC>(dst, src, index);
            break;
          case 6:
            copyWithCacheCtrl<CacheCtrl::L1S_L3C,
                              CacheCtrl::L1S_L3WB>(dst, src, index);
            break;
          case 7:
            copyWithCacheCtrl<CacheCtrl::L1IAR_L3C,
                              CacheCtrl::L1WB_L3WB>(dst, src, index);
            break;
#endif
          }
        });
  }).wait();

  queue.copy(dst, output.data(), elementCount).wait();
  sycl::free(src, queue);
  sycl::free(dst, queue);

  if (output != input) {
#if defined(__SYCL_TARGET_INTEL_GPU_CRI__)
    std::cerr << "CRI cache-control copy failed\n";
#else
    std::cerr << "Xe2 cache-control copy failed\n";
#endif
    return 1;
  }

#if defined(__SYCL_TARGET_INTEL_GPU_CRI__)
  std::cout << "CRI cache-control copy passed\n";
#else
  std::cout << "Xe2 cache-control copy passed\n";
#endif
  return 0;
}
