#pragma once

#define xstr(s) str(s)
#define str(x) #x

// enumeration for all Sends
template <int DataWidth, int DestRegNumber, DataShuffle Transpose, CacheCtrl Cache>
struct RawSendLoad {
  template <typename T, typename AddressPayload>
  static inline void run(T& target, const AddressPayload& adrs);
};

template <int DataWidth, int DestRegNumber, DataShuffle Transpose, CacheCtrl Cache>
struct RawSendStore {
  template <typename T, typename AddressPayload>
  static inline void run(const AddressPayload& adrs, const T& target);
};

#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)

#define EnumerateLoads(DataWidth, DestRegNumber, DataShuffle, CacheCtrl, DescStr) \
  template <> \
  struct RawSendLoad<DataWidth, DestRegNumber, DataShuffle, CacheCtrl> {  \
    template <typename T, typename AddressPayload> \
    static inline void run(T& target, const AddressPayload& address) { \
      asm volatile ("\n"  \
          "raw_sends.15.1.0." str(DestRegNumber) " (M1, 1) 0x0:ud " str(DescStr) ":ud %1.0 V0.0 %0.0\n"  \
        : "=rw"(target) : "rw"(address.getPayload()));  \
    } \
  };

#include "list_raw_loads.list"

#define EnumerateStores(DataWidth, DestRegNumber, DataShuffle, CacheCtrl, DescStr) \
  template <> \
  struct RawSendStore<DataWidth, DestRegNumber, DataShuffle, CacheCtrl> { \
    template <typename T, typename AddressPayload> \
    static inline void run(const AddressPayload& address, const T& target) { \
      asm volatile ("\n"  \
          "raw_sends.15.1." str(DestRegNumber) ".0 (M1, 1) 0x0:ud " str(DescStr) ":ud %0.0 %1.0 V0.0\n"  \
          :: "rw"(address.getPayload()), "rw"(target));  \
    } \
  };

#include "list_raw_stores.list"

#endif
