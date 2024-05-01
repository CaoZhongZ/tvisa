#pragma once

template <typename T, int NRanks, int SubGroupSize>
class BisectTransmit {
  constexpr static int BiNRanks = NRanks / 2;
  constexpr static int NPeers = BiNRanks -1;
  constexpr static int nReg128B = 128 / SubGroupSize / 4;
  constexpr static int firstElem = 0;
  constexpr static int lastElem = nReg128B -1;

protected:
  using message_t = sycl::vec<uint32_t, nReg128B>;
  // transaction of 128-byte is not atomic across HBM channel
  constexpr static int nChan8B = 8 / sizeof(message_t);
  constexpr static int lastDataChannel = SubGroupSize -nChan8B;
  constexpr static int firstFlagChannel = SubGroupSize/2 -1;
  constexpr static int lastFlagChannel = SubGroupSize -1;

  constexpr static int wireCapacity = (SubGroupSize-nChan8B) * sizeof(message_t);
  constexpr static int wireTransSize = SubGroupSize * sizeof(message_t);

  constexpr static size_t wireElems = wireCapacity /sizeof(T);
  constexpr static size_t wireTransElems = wireTransSize /sizeof(T);

private:
  // factor basic communication into another class
  template <int unroll> inline void loadInput(
      message_t (&v)[unroll], T* src, int nElt
  ) {
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    auto lid = sg.get_local_id()[0];
    int local_off = lid * sizeof(message_t) / sizeof(T);

    if (lid < lastDataChannel) { // TODO: diverge
#     pragma unroll
      for (int i = 0; i < unroll; ++ i) {
        auto off = i * wireElems + local_off;
        if (off < nElt) {        // TODO: condition branch !
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
          lscLoad<SubGroupSize/*, CacheCtrl::L1UC_L3UC*/>(
              v[i], src + off
          );
#if defined(__enable_sycl_stream__)
          // cout<<"["<<rank<<","<<lid<<"]off: "<<off
          //   <<", src "<<src<<":"<<v[i]<<sycl::endl;
#endif
#else
          (void)off;
#endif
    }}}
  }

  template <int unroll> inline void loadInput(
      message_t (&v)[unroll], T* src
  ) {
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    auto lid = sg.get_local_id()[0];
    int local_off = lid * sizeof(message_t) / sizeof(T);

    if (lid < lastDataChannel) { // XXX: diverge
#     pragma unroll
      for (int i = 0; i < unroll; ++ i) {
        auto off = i * wireElems + local_off;
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
        lscLoad<SubGroupSize/*, CacheCtrl::L1UC_L3UC*/>(
            v[i], src + off
        );
#else
        (void)off;
#endif
    }}
  }

  template <int unroll>
  inline void insertFlags(
      message_t (& messages)[unroll], uint32_t flag
  ) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
    if constexpr (SubGroupSize == 16) {
#     pragma unroll
      for (int i = 0; i < unroll; ++ i) {
        asm volatile (
            "mov (M1, 1) %0(1, 7)<1> %1(0, 0)<0;1,0>\n"
            : "+rw"(reinterpret_cast<typename message_t::vector_t &>(messages[i]))
            : "rw"(flag)
        );
      }

#     pragma unroll
      for (int i = 0; i < unroll; ++ i) {
        asm volatile (
            "mov (M1, 1) %0(1, 15)<1> %1(0, 0)<0;1,0>\n"
            : "+rw"(reinterpret_cast<typename message_t::vector_t &>(messages[i]))
            : "rw"(flag)
        );
      }
    } else {
#     pragma unroll
      for (int i = 0; i < unroll; ++ i) {
        asm volatile (
            "mov (M1, 1) %0(0, 15)<1> %1(0, 0)<0;1,0>\n"
            : "+rw"(reinterpret_cast<typename message_t::vector_t &>(messages[i])) : "rw"(flag)
        );
      }

#     pragma unroll
      for (int i = 0; i < unroll; ++ i) {
        asm volatile (
            "mov (M1, 1) %0(0, 31)<1> %1(0, 0)<0;1,0>\n"
            : "+rw"(reinterpret_cast<typename message_t::vector_t &>(messages[i])) : "rw"(flag)
        );
      }
    }
#else
    // Add flags at the middle and tail
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    auto lid = sg.get_local_id()[0];
    if (lid == firstFlagChannel || lid == lastFlagChannel) {
#     pragma unroll
      for (int i = 0; i < unroll; ++ i)
        messages[i][lastElem] = flag;
    }
#endif
  }

  template <int unroll>
  static inline void shuffleData(message_t (& messages)[unroll]) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
#   pragma unroll
    for (int i = 0; i < unroll; ++ i) {
      if constexpr (SubGroupSize == 16) {
        asm volatile ("\n"
            "mov (M1, 1) %0(0, 15)<1> %0(1, 7)<0;1,0>\n"
            : "+rw"(reinterpret_cast<typename message_t::vector_t &>(messages[i]))
            :
        );
      } else {
        asm volatile ("\n"
            "mov (M1, 1) %0(0, 30)<1> %0(0, 15)<0;1,0>\n"
            : "+rw"(reinterpret_cast<typename message_t::vector_t &>(messages[i]))
            :
        );
      }
    }
#else
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
#   pragma unroll
    for (int i = 0; i < unroll; ++ i) {
      auto data = sg.shuffle(messages[i][lastElem], SubGroupSize /2 -1);
      if (sg.get_local_id() == lastDataChannel)
        messages[i][firstElem] = data;
    }
#endif
  }

  template <int unroll>
  static inline void restoreData(message_t (& messages)[unroll]) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
#   pragma unroll
    for (int i = 0; i < unroll; ++ i) {
      if constexpr (SubGroupSize == 16) {
        asm volatile ("\n"
            "mov (M1, 1) %0(1, 7)<1> %0(0, 15)<0;1,0>\n"
            : "+rw"(reinterpret_cast<typename message_t::vector_t &>(messages[i]))
            :
        );
      } else {
        asm volatile ("\n"
            "mov (M1, 1) %0(0, 15)<1> %0(0, 30)<0;1,0>\n"
            : "+rw"(reinterpret_cast<typename message_t::vector_t &>(messages[i]))
            :
        );
      }
    }
#else
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
#   pragma unroll
    for (int i = 0; i < unroll; ++ i) {
      auto data = sg.shuffle(messages[i][firstElem], lastDataChannel);
      if (sg.get_local_id() == SubGroupSize / 2 -1)
        messages[i][lastElem] = data;
    }
#endif
  }

  template <int unroll> inline void storeOutput(
      T* dst, message_t (&v)[unroll]
  ) {
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    auto lid = sg.get_local_id()[0];
    int local_off = lid * sizeof(message_t) / sizeof(T);
    if (lid < lastDataChannel) { // XXX: Diverge
#     pragma unroll
      for (int i = 0; i < unroll; ++ i) {
        auto off = i * wireElems + local_off;
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
        lscStore<SubGroupSize/*, CacheCtrl::L1UC_L3UC*/>(
            dst + off, v[i]
        );
#else
        (void)off; (void)local_off;
#endif
    }}
  }

  template <int unroll> inline void storeOutput(
      T* dst, message_t (&v)[unroll], int nElt
  ) {
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    auto lid = sg.get_local_id()[0];
    int local_off = lid * sizeof(message_t) / sizeof(T);
    if (lid < lastDataChannel) { // XXX: Fixed diverge
#     pragma unroll
      for (int i = 0; i < unroll; ++ i) {
        auto off = i * wireElems + local_off;
        if (off < nElt) {        // XXX: runtime condition
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
          lscStore<SubGroupSize/*, CacheCtrl::L1UC_L3UC*/>(
              dst + off, v[i]
          );
#endif
    }}}
  }

  // We always push 128-byte packages
  template <int unroll>
  inline void sendMessages(T* ptr, message_t (&messages)[unroll]) {
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    auto lid = sg.get_local_id()[0];
    int local_off = lid * sizeof(message_t) / sizeof(T);

#   pragma unroll
    for (int u = 0; u < unroll; ++ u) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
      lscStore<SubGroupSize, CacheCtrl::L1UC_L3UC>(
          ptr + u * wireTransElems + local_off,
          messages[u]
      );
#else
      (void) lid; (void) local_off;
#endif
    }
  }

  template <int unroll>
  inline bool recvMessages(message_t (&messages)[unroll], T* ptr, uint32_t flag) {
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    auto lid = sg.get_local_id()[0];
    int local_off = lid * sizeof(message_t) / sizeof(T);

    bool retry = false;
#   pragma unroll
    for (int u = 0; u < unroll; ++ u) {
#if defined(__SYCL_DEVICE_ONLY__) && defined(__SPIR__)
      lscLoad<SubGroupSize, CacheCtrl::L1UC_L3UC>(
          messages[u],
          ptr + u * wireTransElems + local_off
      );
#else
      (void) lid; (void) local_off;
#endif
      retry |= (lid == firstFlagChannel && messages[u][lastElem] != flag)
        || (lid == lastFlagChannel && messages[u][lastElem] != flag);
    }
    return retry;
  }

  template <int unroll> inline void accumMessages(
      message_t (&v)[unroll], message_t (&m)[unroll]
  ) {
    using math_t = sycl::vec<T, sizeof(message_t)/sizeof(T)>;
    auto arith_v = reinterpret_cast<math_t (&)[unroll]>(v);
    auto arith_m = reinterpret_cast<math_t (&)[unroll]>(m);
#   pragma unroll
    for (int u = 0; u < unroll; ++ u)
      arith_v[u] += arith_m[u];
  }
public:

  template <int unroll> inline void scatterFar(
      size_t inputOffset, size_t sinkOffset, ssize_t workLeft
  ) {
    auto inputOffInType = inputOffset / sizeof(T);
    auto sinkOffInType = sinkOffset / sizeof(T);
    auto nelems = workLeft / sizeof(T);

    constexpr auto eltPerPack = unroll * wireElems;

    message_t messages[BiNRanks][unroll];

    if (nelems < eltPerPack) {
      for (int i = 0; i < BiNRanks; ++ i) {
        auto *ptr = ioForFar[i] + inputOffInType;
        loadInput(messages[i], ptr, nelems);
      }
    } else {
      for (int i = 0; i < BiNRanks; ++ i) {
        auto *ptr = ioForFar[i] + inputOffInType;
        loadInput(messages[i], ptr);
      }
    }

#   pragma unroll
    for (int i = 0; i < BiNRanks; ++ i) {
      shuffleData(messages[i]);
      insertFlags(messages[i], farScatterStep);
      auto* dst = farScatterSink[i] + sinkOffInType;
      sendMessages(dst, messages[i]);
    }
  }

  template <int unroll> inline void closePollReduceScatter(
      size_t inputOffset, size_t sinkOffset, ssize_t workLeft
  ) {
    auto inputOffInType = inputOffset / sizeof(T);
    auto sinkOffInType = sinkOffset / sizeof(T);
    auto nelems = workLeft / sizeof(T);
    constexpr auto eltPerPack = unroll * wireElems;

    message_t v[NPeers][unroll];

    // Load all locals
    if (nelems < eltPerPack)
#     pragma unroll
      for (int i = 0; i < NPeers; ++ i)
        loadInput(v[i], ioForPeers[i] + inputOffInType, nelems);
    else
#     pragma unroll
      for (int i = 0; i < NPeers; ++ i)
        loadInput(v[i], ioForPeers[i] + inputOffInType);

    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    // Poll, reduce and send to close remotes
#   pragma unroll
    for (int i = 0; i < NPeers; ++ i) {
      message_t messages[unroll];

      bool retry;
      do {
        retry = false;
        retry |= recvMessages(
            messages, localFarScatterSink[i] + sinkOffInType, farScatterStep);
      } while(sycl::any_of_group(sg, retry));

      shuffleData(v[i]);
      accumMessages(v[i], messages);
      insertFlags(v[i], closeScatterStep);

      sendMessages(scatterSink[i] + sinkOffInType, v[i]);
    }
  }

  template <int unroll>
  inline void closePollRecvReduceBcast(
      size_t inputOffset, size_t sinkOffset, ssize_t workLeft
  ) {
    message_t v[unroll];  //input
    message_t messages[unroll];

    auto nelems = workLeft / sizeof(T);
    auto inputOffInType = inputOffset / sizeof(T);
    auto sinkOffInType = sinkOffset / sizeof(T);

    auto inPtr = ioBuffer + inputOffInType;
    constexpr auto eltPerPack = unroll * wireElems;

    if (nelems < eltPerPack) {
      loadInput(v, inPtr, nelems);
    } else {
      loadInput(v, inPtr);
    }
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
    bool retry;
    do {
      retry = false;
      retry |= recvMessages(
          messages, localFarScatterSink0 + sinkOffInType, farScatterStep);
    } while (sycl::any_of_group(sg, retry));

    shuffleData(v);
    accumMessages(v, messages);

    for (int i =0; i < NPeers; ++ i) {
      bool retry;
      do {
        retry = false;
        retry |= recvMessages(
            messages, localScatterSink[i] + sinkOffInType, closeScatterStep);
      } while (sycl::any_of_group(sg, retry));
      accumMessages(v, messages);
    }

    insertFlags(v, closeGatherStep);
    sendMessages(farGatherSink0 + sinkOffInType, v);

#   pragma unroll
    for (int i = 0; i < NPeers; ++ i)
      sendMessages(gatherSink[i] + sinkOffInType, v);

    restoreData(v);

    // write back to ioBuffer
    if (nelems < eltPerPack) {
      storeOutput(inPtr, v, nelems);
    } else {
      storeOutput(inPtr, v);
    }
  }

  template <int unroll> inline void pollGatherOutputs(
      size_t inputOffset, size_t sinkOffset, ssize_t workLeft
  ) {
    auto inputOffInType = inputOffset / sizeof(T);
    auto sinkOffInType = sinkOffset / sizeof(T);
    auto nelems = workLeft / sizeof(T);

    constexpr auto eltPerPack = unroll * wireElems;
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
#   pragma unroll
    for (int i = 0; i < NPeers; ++ i) {
      bool retry;
      message_t messages[unroll];
      do {
        retry = false;
        retry |= recvMessages(
            messages, localGatherSink[i] + sinkOffInType, closeGatherStep);
      } while(sycl::any_of_group(sg, retry));

      sendMessages(farGatherSink[i] + sinkOffInType, messages);

      restoreData(messages);

      auto* ptr = ioForPeers[i] + inputOffInType;
      if (nelems < eltPerPack)
        storeOutput(ptr, messages, nelems);
      else
        storeOutput(ptr, messages);
    }
  }

  template <int unroll> inline void pollFarGatherOutput(
      size_t inputOffset, size_t sinkOffset, ssize_t workLeft
  ) {
    auto inputOffInType = inputOffset / sizeof(T);
    auto sinkOffInType = sinkOffset / sizeof(T);
    auto nelems = workLeft / sizeof(T);

    constexpr auto eltPerPack = unroll * wireElems;
    auto sg = sycl::ext::oneapi::experimental::this_sub_group();
#   pragma unroll
    for (int i = 0; i < BiNRanks; ++ i) {
      message_t messages[unroll];
      bool retry;
      do {
        retry = false;
        retry |= recvMessages(
            messages, localFarGatherSink[i] + sinkOffInType, closeGatherStep);
      } while(sycl::any_of_group(sg, retry));

      restoreData(messages);

      if (nelems < eltPerPack)
        storeOutput(ioForFar[i] + inputOffInType, messages, nelems);
      else
        storeOutput(ioForFar[i] + inputOffInType, messages);
    }
  }

protected:
  BisectTransmit(
      T* input,
      T* scatterBuf, T* gatherBuf,
      T* const peerBuf0[], T* const peerBuf1[],
      size_t workSize, size_t transmitSize,
      int rank, uint32_t seqNo
#if defined(__enable_sycl_stream__)
      , sycl::stream cout
#endif
  ) : farScatterStep(seqNo), closeScatterStep(seqNo + 1), closeGatherStep(seqNo + 2),
  rank(rank), l_rank(rank/2)
#if defined(__enable_sycl_stream__)
      , cout(cout)
#endif
  {
    auto pairRank = rank ^ 1;

    auto* pairBuf0 = peerBuf0[pairRank];
    auto* pairBuf1 = peerBuf1[pairRank];

    auto ioClosePart = [&](T* p) {
      return (rank & 1) ? (T *)((uintptr_t)p + workSize * BiNRanks) : p;
    };
    auto ioFarPart = [&](T* p) {
      return (rank & 1) ? p : (T *)((uintptr_t)p + workSize * BiNRanks);
    };
    auto ipcClosePart = [&](T *p) {
      return p;
    };
    auto ipcFarPart = [&](T* p) {
      return (T *)((uintptr_t)p + transmitSize * BiNRanks);
    };

    ioBuffer = (T *)((uintptr_t)ioClosePart(input) + l_rank * workSize);
    farGatherSink0 = (T *)((uintptr_t)ipcFarPart(pairBuf1)
        + l_rank * transmitSize);
    localFarScatterSink0 = (T *)((uintptr_t)ipcFarPart(scatterBuf)
        + l_rank * transmitSize);

    for (int i = 0; i < NPeers; ++ i) {
      int l_next = (l_rank + i + 1) % BiNRanks;
      int next = (rank + i * 2 + 2) % (2 * BiNRanks);

      ioForPeers[i] = (T *)((uintptr_t)ioClosePart(input)
          + l_next * workSize);

      localScatterSink[i] = (T *)((uintptr_t)ipcClosePart(scatterBuf)
          + l_next * transmitSize);
      localGatherSink[i] = (T *)((uintptr_t)ipcClosePart(gatherBuf)
          + l_next * transmitSize);

      scatterSink[i] = (T *)((uintptr_t)ipcClosePart(peerBuf0[next])
          + l_rank * transmitSize);
      gatherSink[i] = (T *)((uintptr_t)ipcClosePart(peerBuf1[next])
          + l_rank * transmitSize);

      farGatherSink[i] = (T *)((uintptr_t)ipcFarPart(pairBuf1)
          + l_next * transmitSize);
      localFarScatterSink[i] = (T *)((uintptr_t)ipcFarPart(scatterBuf)
          + l_next * transmitSize);
    }

    for (int i = 0; i < BiNRanks; ++ i) {
      int l_next = (l_rank + i) % BiNRanks;

      ioForFar[i] = (T *)((uintptr_t)ioFarPart(input)
          + l_next * workSize);
      farScatterSink[i] = (T *)((uintptr_t)ipcFarPart(pairBuf0)
          + l_next * transmitSize);
      localFarGatherSink[i] = (T *)((uintptr_t)ipcFarPart(gatherBuf)
          + l_next * transmitSize);
    }
  }

  void dumpOffsets(
      T* input,
      T* scatterBuf, T* gatherBuf,
      T* const peerBuf0[], T* const peerBuf1[]
  ) const {
    std::cout<<std::hex<<"IO offsets: "<<ioBuffer - input;

    for (int i = 0; i < NPeers; ++ i)
      std::cout<<", "<<ioForPeers[i] - input;
    for (int i = 0; i < BiNRanks; ++ i)
      std::cout<<", "<<ioForFar[i] - input;

    std::cout<<"\nLocal IPC sink offsets: "<<localFarScatterSink0 - scatterBuf;
    for (int i = 0; i < NPeers; ++ i)
      std::cout<<", "<<localFarScatterSink[i] - scatterBuf;
    for (int i = 0; i < BiNRanks; ++ i)
      std::cout<<", "<<localFarGatherSink[i] - gatherBuf;

    for (int i = 0; i < NPeers; ++ i)
      std::cout<<", "<<localScatterSink[i] - scatterBuf;
    for (int i = 0; i < NPeers; ++ i)
      std::cout<<", "<<localGatherSink[i] - gatherBuf;

    std::cout<<"\nIPC far offsets: ";

    for (int i = 0; i < BiNRanks; ++ i)
      std::cout<<farScatterSink[i] - peerBuf0[rank ^ 1]<<", ";

    std::cout<<farGatherSink0 - peerBuf1[rank ^ 1];
    for (int i = 0; i < NPeers; ++ i)
      std::cout<<", "<<farGatherSink[i] - peerBuf1[rank ^ 1];

    std::cout<<"\nIPC close offsets: ";
    for (int i = 0;i < NPeers; ++ i)
      std::cout<<scatterSink[i] - peerBuf0[(l_rank + i + 1) % BiNRanks]<<", ";
    for (int i = 0;i < NPeers; ++ i)
      std::cout<<gatherSink[i] - peerBuf1[(l_rank + i + 1) % BiNRanks]<<", ";
  }

  // --------------------Input/Output buffer-------------------
  // Partition close
  T* ioBuffer;
  T* ioForPeers[NPeers];

  // Partition Far
  T* ioForFar[BiNRanks];

  // ---------------------IPC buffers-------------------------
  T* farScatterSink[BiNRanks];

  T* farGatherSink0;
  T* farGatherSink[NPeers];

  T* scatterSink[NPeers];
  T* gatherSink[NPeers];

  // ----------------Sinks, written by remotes----------------
  T* localFarScatterSink0;
  T* localFarScatterSink[NPeers];
  T* localFarGatherSink[BiNRanks];

  T* localScatterSink[NPeers];
  T* localGatherSink[NPeers];

  uint32_t farScatterStep;
  uint32_t closeScatterStep;
  uint32_t closeGatherStep;

  int rank;
  int l_rank;
  uint32_t seqNo;

#if defined(__enable_sycl_stream__)
  sycl::stream cout;
#endif
};
