#pragma once

// sub-group level parallel transmit, minimum latency expected

template <typename T, int NRanks,
         template <typename, int> class Proto, int SubGroupSize = 16>
class ParallelTransmit : public Proto<T, SubGroupSize> {
protected:
  static constexpr int NPeers = NRanks -1;
  using ProtoT = Proto<T, SubGroupSize>;

  using typename ProtoT::message_t;
  using ProtoT::wireCapacityInType;

  using ProtoT::wireTransSize;
  using ProtoT::wireTransElems;

  using ProtoT::loadInput;
  using ProtoT::shuffleData;
  using ProtoT::insertFlags;
  using ProtoT::sendMessages;
  using ProtoT::recvMessages;
  using ProtoT::accumMessages;
  using ProtoT::restoreData;
  using ProtoT::storeOutput;

public:
  constexpr static size_t nSlot = 8;
  constexpr static size_t ringSize = wireTransSize * nSlot;
  constexpr static size_t maxLaunch = 64 * 64;

  typedef T (* ringPtr)[nSlot][wireTransElems];

public:
  ParallelTransmit(
      T* input, T* scatterBuf, T* gatherBuf,
      T* const peerBuf0[], T* const peerBuf1[],
      ssize_t workSize,
      int rank,
      uint32_t seqNo   // Serve as flag for checking
#if defined(__enable_sycl_stream__)
      , sycl::stream cout
#endif
  ) : workElems(workSize/sizeof(T)), rank(rank), seqNo(seqNo)
#if defined(__enable_sycl_stream__)
  , cout(cout)
#endif
  {
    ioForPeers = input;

    localGatherSink = reinterpret_cast<ringPtr>(gatherBuf);

    for (int i = 0; i < NRanks; ++ i)
      scatterSink[i] = reinterpret_cast<ringPtr>(
          (uintptr_t)peerBuf0[i] + rank * ringSize);

    for (int i = 0; i < NPeers; ++ i) {
      int next = (rank + i + 1) % NRanks;

      localScatterSink[i] = reinterpret_cast<ringPtr>(
          (uintptr_t)scatterBuf + next * ringSize);

      gatherSink[i] = reinterpret_cast<ringPtr>(
          (uintptr_t)peerBuf1[next] + rank * ringSize);
    }
  }

  template <int unroll>
  inline void run(
      size_t inputOffset, size_t tStep, ssize_t workLeft
  ) {
    auto wireId = sycl::ext::oneapi::experimental::
      this_nd_item<1>().get_global_id(0) / SubGroupSize;

    auto y_id = wireId % NRanks;
    auto x_id = wireId / NRanks * NRanks;

    auto inputOffInType = inputOffset / sizeof(T);
    auto flag = seqNo + tStep / nSlot;
    auto nelems = workLeft / sizeof(T);

    constexpr auto eltPerPack = unroll * wireCapacityInType;

    message_t v[unroll];

    auto* ptr = ioForPeers + y_id * workElems + inputOffInType;

    if (nelems < eltPerPack)
      loadInput(v, ptr, nelems);
    else
      loadInput(v, ptr);

    shuffleData(v);

    if (y_id != rank) { // scatter and gather
      insertFlags(v, flag);
      sendMessages(scatterSink[y_id][x_id][tStep%nSlot], v);

      bool retry;
      do {
        retry = false;
        retry |= recvMessages(v, localGatherSink[wireId][tStep%nSlot], flag);
      } while(sycl::any_of_group(
            sycl::ext::oneapi::experimental::this_sub_group(), retry)
        );
    }

    if (y_id == rank) { // wait reduce bcast
      message_t messages[unroll];
      for (int i = 0; i < NPeers; ++ i) {
        bool retry;
        do {
          retry = false;
          retry |= recvMessages(
              messages, localScatterSink[i][x_id][tStep%nSlot], flag);
        } while (sycl::any_of_group(
              sycl::ext::oneapi::experimental::this_sub_group(), retry)
          );
        accumMessages(v, messages);
      }

      insertFlags(v, flag);

#     pragma unroll
      for (int i = 0; i < NPeers; ++ i)
        sendMessages(gatherSink[i][x_id][tStep%nSlot], v);
    }

    restoreData(v);

    if (nelems < eltPerPack)
      storeOutput(ptr, v, nelems);
    else
      storeOutput(ptr, v);
  }

protected:
  T* ioForPeers;

  ssize_t workElems;

  int rank;
  uint32_t seqNo;

  ringPtr scatterSink[NRanks];
  ringPtr gatherSink[NPeers];

  ringPtr localScatterSink[NPeers];
  ringPtr localGatherSink;

#if defined(__enable_sycl_stream__)
  sycl::stream cout;
#endif
};


