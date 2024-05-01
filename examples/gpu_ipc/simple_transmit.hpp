#pragma once

template <typename T, int NPeers, int SubGroupSize,
         template <typename, int> class Proto = Rt64_128>
class SimpleTransmit : public Proto<T, SubGroupSize> {
public:
  //
  // sectionSize will be renamed later, it represent each temporary buffer
  // section for each rank. configurable, in bytes
  //
  constexpr static size_t sectionSize = 0x200000;
  constexpr static size_t sectionElems = sectionSize / sizeof(T);
  constexpr static size_t scratchSize = alignUp(sectionSize * (NPeers + 1), 0x200000);

public:
  SimpleTransmit(
      T* input,
      T* scatterBuf, T* gatherBuf,
      T* const peerBuf0[], T* const peerBuf1[],
      ssize_t workSize,
      int rank,
      uint32_t seqNo  // Serve as flag for checking
#if defined(__enable_sycl_stream__)
      , sycl::stream cout
#endif
  ) : seqNo(seqNo), rank(rank)
#if defined(__enable_sycl_stream__)
  , cout(cout)
#endif
  {
    ioBuffer = (input + rank * workSize / sizeof(T));

    for (int i = 0; i < NPeers; ++ i) {
      int next = (rank + i + 1) % (NPeers + 1);

      scatterSink[i] = (T *)((uintptr_t)peerBuf0[next]
          + sectionSize * rank);
      gatherSink[i] = (T *)((uintptr_t)peerBuf1[next]
          + sectionSize * rank);

      localScatterSink[i] = (T *)((uintptr_t)scatterBuf
          + next * sectionSize);
      localGatherSink[i] = (T *)((uintptr_t)gatherBuf
          + next * sectionSize);

      ioForPeers[i] = input + next * workSize / sizeof(T);
    }
  }
  static inline size_t ringOffset(size_t sinkOffset) {
    return sinkOffset % sectionSize;
  }

  static inline int seqDelta(size_t sinkOffset) {
    return sinkOffset / sectionSize;
  }

  // Scatter local message to peers
  template <int unroll>
  inline void scatter(
      size_t inputOffset, size_t sinkOffset, ssize_t workLeft
  ) {
    auto inputOffInType = inputOffset / sizeof(T);
    auto sinkOffInType = ringOffset(sinkOffset) / sizeof(T);
    auto flag = seqNo + seqDelta(sinkOffset);
    auto nelems = workLeft / sizeof(T);

    constexpr auto eltPerPack = unroll * wireCapacityInType;
    //
    // register consumption:
    // 2 x unroll x NPeers;
    //
    // SWSB consumption:
    // unroll * NPeers;
    //
    static_assert(unroll * NPeers * 2 < 64, "Too many registers consumed");
    static_assert(NPeers * 2 < 16, "Too many swsb consumed");
    message_t messages[NPeers][unroll];

    if (nelems < eltPerPack) {
#     pragma unroll
      for (int i = 0; i < NPeers; ++ i) {
        auto* ptr = ioForPeers[i] + inputOffInType;
        loadInput(messages[i], ptr, nelems);
      }
    } else {
      // Fast path. No predicated load
#     pragma unroll
      for (int i = 0; i < NPeers; ++ i) {
        auto* ptr = ioForPeers[i] + inputOffInType;
        loadInput(messages[i], ptr);
      }
    }

#   pragma unroll
    for (int i = 0; i < NPeers; ++ i) {
      shuffleData(messages[i]);
      insertFlags(messages[i], flag);

      auto* dst = scatterSink[i] + sinkOffInType;
      sendMessages(dst, messages[i]);
    }
  }

  template <int unroll>
  inline void pollRecvReduceBcast(
      size_t inputOffset, size_t sinkOffset, ssize_t workLeft
  ) {
    message_t v[unroll];        // Input
    message_t messages[unroll]; // Scraps from remote

    auto nelems = workLeft / sizeof(T);
    auto inputOffInType = inputOffset / sizeof(T);
    auto sinkOffInType = ringOffset(sinkOffset) / sizeof(T);
    auto flag = seqNo + seqDelta(sinkOffset);

    auto inPtr = ioBuffer + inputOffInType;
    constexpr auto eltPerPack = unroll * wireCapacityInType;

    if (nelems < eltPerPack) {
      loadInput(v, inPtr, nelems);
    } else {
      loadInput(v, inPtr);
    }

    auto sg = sycl::ext::oneapi::experimental::this_sub_group();

#   pragma unroll
    for (int i = 0; i < NPeers; ++ i) {
      bool retry;
      do {
        retry = false;
        retry |= recvMessages(messages, localScatterSink[i] + sinkOffInType, flag);
      } while(sycl::any_of_group(sg, retry));

#if defined(__enable_sycl_stream__)
      if (lane_id == firstFlagChannel || lane_id == lastFlagChannel) {
        cout<<"["<<rank<<","<<lane_id<<"]";
        for (int u = 0; u < unroll; ++ u)
          cout<<sycl::hex<<messages[u]<<"; ";
        cout<<sycl::endl<<sycl::flush;
      }
#endif

      restoreData(messages);
      accumMessages(v, messages);
    }

    // write back locally before shuffle data
    if (nelems < eltPerPack) {
      storeOutput(inPtr, v, nelems);
    } else {
      storeOutput(inPtr, v);
    }

    shuffleData(v);
    insertFlags(v, flag);

    // push to gather sinks
#   pragma unroll
    for (int i = 0; i < NPeers; ++ i)
      sendMessages(gatherSink[i] + sinkOffInType, v);
  }

  template <int unroll>
  inline void pollGatherOutputs(
      size_t inputOffset, size_t sinkOffset, ssize_t workLeft
  ) {
    auto inputOffInType = inputOffset / sizeof(T);
    auto sinkOffInType = ringOffset(sinkOffset) / sizeof(T);
    auto flag = seqNo + seqDelta(sinkOffset);
    auto nelems = workLeft / sizeof(T);

    constexpr auto eltPerPack = unroll * wireCapacityInType;
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();

#   pragma unroll
    for (int i = 0; i < NPeers; ++ i) {
      bool retry;
      message_t messages[unroll];
      do {
        retry = false;
        retry |= recvMessages(messages, localGatherSink[i] + sinkOffInType, flag);
      } while(sycl::any_of_group(sg, retry));

      auto ptr = ioForPeers[i] + inputOffInType;

      restoreData(messages);

      if (nelems < eltPerPack)
        storeOutput(ptr, messages, nelems);
      else
        storeOutput(ptr, messages);
    }
  }

protected:
  T* scatterSink[NPeers];
  T* gatherSink[NPeers];
  T* localScatterSink[NPeers];
  T* localGatherSink[NPeers];
  T* ioBuffer; // point to workload of self
  T* ioForPeers[NPeers]; // point to distributed workload

  uint32_t seqNo;
  int rank;

#if defined(__enable_sycl_stream__)
  sycl::stream cout;
#endif
};
