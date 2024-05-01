#include "utils.hpp"
#include "allreduce.hpp"

template <typename T>
static void allreduce(T* allRanks[], int nRanks, size_t nelems) {
# pragma omp parallel for
  for (int i = 0; i < nelems; ++ i) {
    T sum = 0.0;
    for (int r = 0; r < nRanks; ++ r)
      sum += allRanks[r][i];

    for (int r = 0; r < nRanks; ++ r)
      allRanks[r][i] = sum;
  }
}

template <typename T>
static void allreduce(T* result, T* allRanks[], int nRanks, size_t nelems) {
# pragma omp parallel for
  for (int i = 0; i < nelems; ++ i) {
    T sum = 0.0;
    for (int r = 0; r < nRanks; ++ r)
      sum += allRanks[r][i];
    result[i] = sum;
  }
}

// template <typename T,
//          int NPeers,
//          template <typename, int, int> class Transmit,
//          int SubGroupSize>
// int AllReduce<T, NPeers, Transmit, SubGroupSize>::scatterVerify(
//     uint32_t* host, int rank, uint32_t flag, size_t nWorkElemsInInt
// ){
//   constexpr auto n120B = 120 / 4;
//   constexpr auto wireCapInInt = wireCapacity / sizeof(uint32_t);
//   constexpr auto wireTransInInt = wireTransSize / sizeof(uint32_t);
//
//   auto nTransmitElemsInInt
//     = divUp(nWorkElemsInInt, wireCapInInt) * wireTransInInt;
//
//   for (int i = 0; i < NPeers; ++ i) {
//     int next = (rank + i + 1) % (NPeers + 1);
//     auto* peer_ptr = host + nTransmitElemsInInt * next;
//     size_t contentOff = rank * nWorkElemsInInt;
//
//     // we are expecting pattern = (scale | next)
//     size_t nChunks = divUp(nWorkElemsInInt, wireCapInInt);
//     for (int chunk = 0; chunk < nChunks; ++ chunk) {
//       uint32_t temp[32];
//       uint32_t scrat[32];
//
//       for (size_t b = 0, j = 0; b < wireCapInInt; ++ b, ++ j) {
//         if (b + chunk * wireCapInInt < nWorkElemsInInt)
//           temp[b % n120B] = (b + chunk * wireCapInInt + contentOff) % 32 | next << 16;
//         else
//           temp[b % n120B] = 0xffffffff;
//         scrat[j % 32] = peer_ptr[j + chunk * wireTransInInt];
//
//         // wireCapInInt will be divided by n120B.
//         if (b % n120B == n120B -1) {
//           temp[30] = temp[15]; temp[15] = flag; temp[31] = flag;
//           scrat[30] = peer_ptr[++j + chunk * wireTransInInt];
//           scrat[31] = peer_ptr[++j + chunk * wireTransInInt];
//
//           for (auto k = 0; k < 32; ++ k) {
//             if (temp[k] != scrat[k] && temp[k] != 0xffffffff) {
//               std::cout<<"["<<rank<<"] Verify failed @"<<i<<", "<<k
//                 <<", expect:"<<temp[k]<<", but get:"<<scrat[k]<<std::endl;
//               return -1;
//     }}}}}
//   }
//
//   return 0;
// }

// template <typename T,
//          int NRanks,
//          template <typename, int, int> class Transmit,
//          int SubGroupSize>
// int bisectAllReduce<T, NRanks, Transmit, SubGroupSize>::stage1Verify(
//     T* host, int rank, uint32_t flag, size_t nelems
// ){
//   constexpr auto wireCapInType = wireCapacity / sizeof(T);
//   constexpr auto wireTransInType = wireTransSize / sizeof(T);
//
//   auto nWorkElems = nelems / NRanks;
//   auto nWorkElemsInInt = nWorkElems * sizeof(T) / sizeof(uint32_t);
//   size_t nChunks = divUp(nWorkElems, wireCapInType);
//   auto nTransmitElems = nChunks * wireTransInType;
//
//   T* allRanks[NRanks];
//   T* allIpcBuffers[NRanks];
//
//   for (int i = 0; i < NRanks; ++ i) {
//     allRanks[i] = (T *)malloc(sizeof(T) * nWorkElems * NRanks);
//     allIpcBuffers[i] = (T *)malloc(sizeof(T) * nTransmitElems * NRanks);
//   }
//
//   __scope_guard free_pointers([&] {
//     for (int i = 0; i < NRanks; ++ i) {
//       free(allRanks[i]);
//       free(allIpcBuffers[i]);
//     }
//   });
//
//   for (int i = 0; i < NRanks; ++ i)
//     fill_pattern(allRanks[i], i, nelems);
//
//   // simulate stage 1
//   auto farScatterSim  = [&](T* ipcBuffer, T* input) {
//     auto* ipcSink = reinterpret_cast<T (*)[nChunks][wireTransInType]>(ipcBuffer);
//
//     for (int r = 0; r < NRanks/2; ++ r) {
//       for (int chunk = 0; chunk < nChunks; ++ chunk) {
//         T ipcTemp[wireTransInType];
//         for (int i = 0; i < wireCapInType; ++ i)
//           ipcTemp[i]
//             = (i + chunk * wireCapInType < nWorkElems) ?
//             input[i + chunk * wireCapInType + r * nWorkElems] : (T)0;
//
//         ((uint32_t *)ipcTemp)[30] = ((uint32_t *)ipcTemp)[15];
//         ((uint32_t *)ipcTemp)[15] = flag;
//         ((uint32_t *)ipcTemp)[31] = flag;
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcSink[r][chunk][i] = ipcTemp[i];
//       }
//     }
//   };
//
//   T* ipcBuffer = allIpcBuffers[rank] + nTransmitElems * NRanks/2;
//   T* host_start = host + nTransmitElems * NRanks /2;
//
//   T* input = (rank & 1) ? allRanks[rank ^ 1] + nelems/2 : allRanks[rank ^ 1];
//
//   farScatterSim(ipcBuffer, input);
//
//   auto compareResult = [&](T * ipcBuffer, T* ipcHost) {
//     constexpr auto wireCapInInt = wireCapacity / sizeof(uint32_t);
//     constexpr auto wireTransInInt = wireTransSize / sizeof(uint32_t);
//
//     auto* ipcBufInt = reinterpret_cast<uint32_t (*)[nChunks][wireTransInInt]>(ipcBuffer);
//     auto* ipcHostInt = reinterpret_cast<uint32_t (*)[nChunks][wireTransInInt]>(ipcHost);
//
//     for (int r = 0; r < NRanks / 2; ++ r)
//     for (int chunk = 0; chunk < nChunks; ++ chunk) {
//       for (int i = 0; i < wireTransInInt; ++ i) {
//         int j = i;
//         if ( i == 30 ) j = 15;
//         if ( i == 15 ) j = 30;
//
//         if ( (j < 30) && j + chunk * wireCapInInt >= nWorkElemsInInt )
//           continue;
//
//         if (ipcBufInt[r][chunk][i] != ipcHostInt[r][chunk][i]) {
//           std::cout<<"Error Compare!"<<std::endl;
//           return -1;
//         }
//       }
//     }
//
//     return 0;
//   };
//
//   return compareResult(ipcBuffer, host_start);
// }

// template <typename T,
//          int NRanks,
//          template <typename, int, int> class Transmit,
//          int SubGroupSize>
// int bisectAllReduce<T, NRanks, Transmit, SubGroupSize>::stage2Verify(
//     T* host, int rank, uint32_t flag, size_t nelems
// ){
//   constexpr auto wireCapInType = wireCapacity / sizeof(T);
//   constexpr auto wireTransInType = wireTransSize / sizeof(T);
//
//   auto nWorkElems = nelems / NRanks;
//   auto nWorkElemsInInt = nWorkElems * sizeof(T) / sizeof(uint32_t);
//   size_t nChunks = divUp(nWorkElems, wireCapInType);
//   auto nTransmitElems = nChunks * wireTransInType;
//
//   T* allRanks[NRanks];
//   T* allIpcBuffers[NRanks];
//
//   for (int i = 0; i < NRanks; ++ i) {
//     allRanks[i] = (T *)malloc(sizeof(T) * nWorkElems * NRanks);
//     allIpcBuffers[i] = (T *)malloc(sizeof(T) * nTransmitElems * NRanks);
//   }
//
//   __scope_guard free_pointers([&] {
//     for (int i = 0; i < NRanks; ++ i) {
//       free(allRanks[i]);
//       free(allIpcBuffers[i]);
//     }
//   });
//
//   for (int i = 0; i < NRanks; ++ i) {
//     fill_pattern(allRanks[i], i, nelems);
//     memset(allIpcBuffers[i], 0, sizeof(T) * nTransmitElems * NRanks);
//   }
//
//   // simulate stage 1
//   auto farScatterSim  = [&](T* ipcBuffer, T* input) {
//     auto* ipcSink = reinterpret_cast<T (*)[nChunks][wireTransInType]>(ipcBuffer);
//
//     for (int r = 0; r < NRanks/2; ++ r) {
//       for (int chunk = 0; chunk < nChunks; ++ chunk) {
//         T ipcTemp[wireTransInType];
//         for (int i = 0; i < wireCapInType; ++ i)
//           ipcTemp[i]
//             = (i + chunk * wireCapInType < nWorkElems) ?
//             input[i + chunk * wireCapInType + r * nWorkElems] : (T)0;
//
//         ((uint32_t *)ipcTemp)[30] = ((uint32_t *)ipcTemp)[15];
//         ((uint32_t *)ipcTemp)[15] = flag + 1;
//         ((uint32_t *)ipcTemp)[31] = flag + 1;
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcSink[r][chunk][i] = ipcTemp[i];
//       }
//     }
//   };
//
//   // run sim on all ranks
//   for (int r = 0; r < NRanks; ++ r) {
//     T* ipcBuffer = allIpcBuffers[r] + nTransmitElems * NRanks/2;
//     T* input = (r & 1) ? allRanks[r ^ 1] + nelems/2 : allRanks[r ^ 1];
//
//     farScatterSim(ipcBuffer, input);
//   }
//
//   // simulate stage 2
//   auto closeScatterSim = [&](T* ipcBuffers[], T* ipcBuffer, T* input, int rank) {
//     auto l_rank = rank /2;
//     for (int r = 0; r < NRanks/2 -1; ++ r) {
//       int l_next = (l_rank + r + 1) % (NRanks/2);
//       int next = (rank + 2 *(r + 1)) % NRanks;
//
//       for (int chunk = 0; chunk < nChunks; ++ chunk) {
//         T inputTemp[wireTransInType];
//         T ipcTemp[wireTransInType];
//
//         for (int i = 0; i < wireCapInType; ++ i) {
//           inputTemp[i]
//             = (i + chunk * wireCapInType < nWorkElems) ?
//             input[i + chunk * wireCapInType + l_next * nWorkElems] : (T)0;
//         }
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcTemp[i] = ipcBuffer[i + chunk * wireTransInType + l_next * nTransmitElems];
//
//         ((uint32_t *)inputTemp)[30] = ((uint32_t *)inputTemp)[15];
//         ((uint32_t *)inputTemp)[15] = 0;
//         ((uint32_t *)inputTemp)[31] = 0;
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcTemp[i] += inputTemp[i];
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcBuffers[next][l_rank * nTransmitElems + chunk * wireTransInType + i] = ipcTemp[i];
//       }
//     }
//   };
//
//   for (int r = 0; r < NRanks; ++ r) {
//     T* ipcBuffer = allIpcBuffers[r] + nTransmitElems * NRanks/2;
//     T* input2 = (r & 1) ? allRanks[r] : allRanks[r] + nelems/2;
//     closeScatterSim(allIpcBuffers, ipcBuffer, input2, r);
//   }
//
//   auto compareResult = [&](T * ipcBuffer, T* ipcHost) {
//     constexpr auto wireCapInInt = wireCapacity / sizeof(uint32_t);
//     constexpr auto wireTransInInt = wireTransSize / sizeof(uint32_t);
//
//     auto* ipcBufInt = reinterpret_cast<uint32_t (*)[nChunks][wireTransInInt]>(ipcBuffer);
//     auto* ipcHostInt = reinterpret_cast<uint32_t (*)[nChunks][wireTransInInt]>(ipcHost);
//
//     for (int r = 0; r < NRanks/2; ++ r) {
//       if (r * 2 == rank)
//         continue;
//
//       for (int chunk = 0; chunk < nChunks; ++ chunk) {
//         for (int i = 0; i < wireTransInInt; ++ i) {
//           int j = i;
//           if ( i == 30 ) j = 15;
//           if ( i == 15 ) j = 30;
//
//           if ( j < 30 && j + chunk * wireCapInInt >= nWorkElemsInInt )
//             continue;
//
//           if (ipcBufInt[r][chunk][i] != ipcHostInt[r][chunk][i]) {
//             std::cout<<"Error Compare! Expect: "<<std::hex
//               <<ipcBufInt[r][chunk][i]<<", got: "
//               <<ipcHostInt[r][chunk][i]<<"@"
//               <<r<<","<<chunk<<","<<i<<std::endl;
//             return -1;
//     }}}}
//
//     return 0;
//   };
//
//   auto ipcBuffer = allIpcBuffers[rank];
//   return compareResult(ipcBuffer, host);
// }

// template <typename T,
//          int NRanks,
//          template <typename, int, int> class Transmit,
//          int SubGroupSize>
// int bisectAllReduce<T, NRanks, Transmit, SubGroupSize>::stage3Verify(
//     T* host, int rank, uint32_t flag, size_t nelems
// ){
//   constexpr auto wireCapInType = wireCapacity / sizeof(T);
//   constexpr auto wireTransInType = wireTransSize / sizeof(T);
//
//   auto nWorkElems = nelems / NRanks;
//   auto nWorkElemsInInt = nWorkElems * sizeof(T) / sizeof(uint32_t);
//   size_t nChunks = divUp(nWorkElems, wireCapInType);
//   auto nTransmitElems = nChunks * wireTransInType;
//
//   T* allRanks[NRanks];
//   T* allIpcBuffers[NRanks];
//   T* allgatherBuffers[NRanks];
//
//   for (int i = 0; i < NRanks; ++ i) {
//     allRanks[i] = (T *)malloc(sizeof(T) * nWorkElems * NRanks);
//     allIpcBuffers[i] = (T *)malloc(sizeof(T) * nTransmitElems * NRanks);
//     allgatherBuffers[i] = (T *)malloc(sizeof(T) * nTransmitElems * NRanks);
//   }
//
//   auto* allreduceResult = (T*)malloc(sizeof(T) * nelems);
//
//   __scope_guard free_pointers([&] {
//     for (int i = 0; i < NRanks; ++ i) {
//       free(allRanks[i]);
//       free(allIpcBuffers[i]);
//       free(allgatherBuffers[i]);
//     }
//     free(allreduceResult);
//   });
//
//   for (int i = 0; i < NRanks; ++ i) {
//     fill_pattern(allRanks[i], i, nelems);
//     memset(allIpcBuffers[i], 0, sizeof(T) * nTransmitElems * NRanks);
//     memset(allgatherBuffers[i], 0, sizeof(T) * nTransmitElems * NRanks);
//   }
//
//   allreduce(allreduceResult, allRanks, NRanks, nelems);
//
//   // simulate stage 1
//   auto farScatterSim  = [&](T* ipcBuffer, T* input) {
//     auto* ipcSink = reinterpret_cast<T (*)[nChunks][wireTransInType]>(ipcBuffer);
//
//     for (int r = 0; r < NRanks/2; ++ r) {
//       for (int chunk = 0; chunk < nChunks; ++ chunk) {
//         T ipcTemp[wireTransInType];
//         for (int i = 0; i < wireCapInType; ++ i)
//           ipcTemp[i]
//             = (i + chunk * wireCapInType < nWorkElems) ?
//             input[i + chunk * wireCapInType + r * nWorkElems] : (T)0;
//
//         ((uint32_t *)ipcTemp)[30] = ((uint32_t *)ipcTemp)[15];
//         ((uint32_t *)ipcTemp)[15] = flag + 1;
//         ((uint32_t *)ipcTemp)[31] = flag + 1;
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcSink[r][chunk][i] = ipcTemp[i];
//       }
//     }
//   };
//
//   // run sim on all ranks
//   for (int r = 0; r < NRanks; ++ r) {
//     T* ipcBuffer = allIpcBuffers[r] + nTransmitElems * NRanks/2;
//     T* input = (r & 1) ? allRanks[r ^ 1] + nelems/2 : allRanks[r ^ 1];
//
//     farScatterSim(ipcBuffer, input);
//   }
//
//   // simulate stage 2
//   auto closeScatterSim = [&](T* ipcBuffers[], T* ipcBuffer, T* input, int rank) {
//     auto l_rank = rank /2;
//     for (int r = 0; r < NRanks/2 -1; ++ r) {
//       int l_next = (l_rank + r + 1) % (NRanks/2);
//       int next = (rank + 2 *(r + 1)) % NRanks;
//
//       for (int chunk = 0; chunk < nChunks; ++ chunk) {
//         T inputTemp[wireTransInType];
//         T ipcTemp[wireTransInType];
//
//         for (int i = 0; i < wireCapInType; ++ i) {
//           inputTemp[i]
//             = (i + chunk * wireCapInType < nWorkElems) ?
//             input[i + chunk * wireCapInType + l_next * nWorkElems] : (T)0;
//         }
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcTemp[i] = ipcBuffer[i + chunk * wireTransInType + l_next * nTransmitElems];
//
//         ((uint32_t *)inputTemp)[30] = ((uint32_t *)inputTemp)[15];
//         ((uint32_t *)inputTemp)[15] = 0;
//         ((uint32_t *)inputTemp)[31] = 0;
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcTemp[i] += inputTemp[i];
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcBuffers[next][l_rank * nTransmitElems + chunk * wireTransInType + i] = ipcTemp[i];
//       }
//     }
//   };
//
//   for (int r = 0; r < NRanks; ++ r) {
//     T* ipcBuffer = allIpcBuffers[r] + nTransmitElems * NRanks/2;
//     T* input2 = (r & 1) ? allRanks[r] : allRanks[r] + nelems/2;
//     closeScatterSim(allIpcBuffers, ipcBuffer, input2, r);
//   }
//
//   // simulate stage 3
//   auto closeBcastSim = [&](T* ipcBuffer, T* biBuffer, T* input, int rank) {
//     auto l_rank = rank /2;
//
//     for (int chunk = 0; chunk < nChunks; ++ chunk) {
//       T inputTemp[wireTransInType];
//       T ipcTemp[wireTransInType];
//
//       for (int i = 0; i < wireCapInType; ++ i)
//         inputTemp[i]
//           = (i + chunk * wireCapInType < nWorkElems) ?
//           input[i + chunk * wireCapInType + l_rank * nWorkElems] : (T)0;
//
//       ((uint32_t *)inputTemp)[30] = ((uint32_t *)inputTemp)[15];
//       ((uint32_t *)inputTemp)[15] = 0;
//       ((uint32_t *)inputTemp)[31] = 0;
//
//       for (int i = 0; i < wireTransInType; ++ i)
//         ipcTemp[i] = biBuffer[i + chunk * wireTransInType + l_rank * nTransmitElems];
//
//       for (int i = 0; i < wireTransInType; ++ i)
//         inputTemp[i] += ipcTemp[i];
//
//       for (int r = 0; r < NRanks/2 -1; ++ r) {
//         int l_next = (l_rank + r + 1) % (NRanks/2);
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           ipcTemp[i] = ipcBuffer[i + chunk * wireTransInType + l_next * nTransmitElems];
//
//         for (int i = 0; i < wireTransInType; ++ i)
//           inputTemp[i] += ipcTemp[i];
//
//       }
//
//       ((uint32_t *)inputTemp)[15] = flag+2;
//       ((uint32_t *)inputTemp)[31] = flag+2;
//
//       for (int r = 0; r < NRanks/2 -1; ++ r) {
//         int next = (rank + 2 *(r + 1)) % NRanks;
//         for (int i = 0; i < wireTransInType; ++ i)
//           allgatherBuffers[next][l_rank * nTransmitElems
//             + chunk * wireTransInType + i] = inputTemp[i];
//       }
//     }
//   };
//
//   for (int r = 0; r < NRanks; ++ r) {
//     T* ipcBuffer = allIpcBuffers[r];
//     T* biBuffer = ipcBuffer + nTransmitElems * NRanks/2;
//     T* input3 = (r & 1) ? allRanks[r] : allRanks[r] + nelems/2;
//
//     closeBcastSim(ipcBuffer, biBuffer, input3, r);
//   }
//
//   auto compareResult = [&](T * ipcBuffer, T* ipcHost) {
//     constexpr auto wireCapInInt = wireCapacity / sizeof(uint32_t);
//     constexpr auto wireTransInInt = wireTransSize / sizeof(uint32_t);
//
//     auto* ipcBufInt = reinterpret_cast<uint32_t (*)[nChunks][wireTransInInt]>(ipcBuffer);
//     auto* ipcHostInt = reinterpret_cast<uint32_t (*)[nChunks][wireTransInInt]>(ipcHost);
//
//     for (int r = 0; r < NRanks/2; ++ r) {
//       if (r * 2 == rank)
//         continue;
//
//       for (int chunk = 0; chunk < nChunks; ++ chunk) {
//         for (int i = 0; i < wireTransInInt; ++ i) {
//           int j = i;
//           if ( i == 30 ) j = 15;
//           if ( i == 15 ) j = 30;
//
//           if ( j < 30 && j + chunk * wireCapInInt >= nWorkElemsInInt )
//             continue;
//
//           if (ipcBufInt[r][chunk][i] != ipcHostInt[r][chunk][i]) {
//             std::cout<<"Error Compare! Expect: "<<std::hex
//               <<ipcBufInt[r][chunk][i]<<", got: "
//               <<ipcHostInt[r][chunk][i]<<"@"
//               <<r<<","<<chunk<<","<<i<<std::endl;
//             return -1;
//     }}}}
//
//     return 0;
//   };
//
//   auto ipcBuffer = allgatherBuffers[rank];
//   return compareResult(ipcBuffer, host + 16 * 1024 * 1024);
// }

template <typename T> int verifyAllReduce(
    T* host, int rank, int world, size_t nelems
){
  T* allRanks[16];

  for (int i = 0; i < world; ++ i) {
    allRanks[i] = (T *)malloc(sizeof(T) * nelems);
  }

  auto* allreduceResult = (T*)malloc(sizeof(T) * nelems);

  __scope_guard free_pointers([&] {
    for (int i = 0; i < world; ++ i) {
      free(allRanks[i]);
    }
    free(allreduceResult);
  });

  for (int i = 0; i < world; ++ i) {
    fill_pattern(allRanks[i], i, nelems);
  }

  allreduce(allreduceResult, allRanks, world, nelems);

  for (int i = 0; i < nelems; ++ i) {
    if (allreduceResult[i] != host[i]) {
      std::cout<<"Error Compare! Expected: "
        <<allreduceResult[i] <<", got: "<<host[i]
        <<"@["<<rank<<"]("<<i<<");"<<std::endl;
    }
  }
  return 0;
}

// template <typename T,
//          int NRanks,
//          template <typename, int, int> class Transmit,
//          int SubGroupSize>
// int bisectAllReduce<T, NRanks, Transmit, SubGroupSize>::stage4Verify(
//     T* host, int rank, uint32_t flag, size_t nelems
// ){
//   T* allRanks[NRanks];
//
//   for (int i = 0; i < NRanks; ++ i) {
//     allRanks[i] = (T *)malloc(sizeof(T) * nelems);
//   }
//
//   auto* allreduceResult = (T*)malloc(sizeof(T) * nelems);
//
//   __scope_guard free_pointers([&] {
//     for (int i = 0; i < NRanks; ++ i) {
//       free(allRanks[i]);
//     }
//     free(allreduceResult);
//   });
//
//   for (int i = 0; i < NRanks; ++ i) {
//     fill_pattern(allRanks[i], i, nelems);
//   }
//
//   allreduce(allreduceResult, allRanks, NRanks, nelems);
//
//   for (int i = 0; i < nelems; ++ i) {
//     if (allreduceResult[i] != host[i]) {
//       std::cout<<"Error Compare! Expected: "
//         <<allreduceResult[i] <<", got: "<<host[i]
//         <<"@["<<rank<<"]("<<i<<");"<<std::endl;
//     }
//   }
//   return 0;
// }

// template <typename T,
//          int NRanks,
//          template <typename, int, int> class Transmit,
//          int SubGroupSize>
// int bisectPAllReduce<T, NRanks, Transmit, SubGroupSize>::stage4Verify(
//     T* host, int rank, uint32_t flag, size_t nelems
// ){
//   T* allRanks[NRanks];
//
//   for (int i = 0; i < NRanks; ++ i) {
//     allRanks[i] = (T *)malloc(sizeof(T) * nelems);
//   }
//
//   auto* allreduceResult = (T*)malloc(sizeof(T) * nelems);
//
//   __scope_guard free_pointers([&] {
//     for (int i = 0; i < NRanks; ++ i) {
//       free(allRanks[i]);
//     }
//     free(allreduceResult);
//   });
//
//   for (int i = 0; i < NRanks; ++ i) {
//     fill_pattern(allRanks[i], i, nelems);
//   }
//
//   allreduce(allreduceResult, allRanks, NRanks, nelems);
//
//   for (int i = 0; i < nelems; ++ i) {
//     if (allreduceResult[i] != host[i]) {
//       std::cout<<"Error Compare! Expected: "
//         <<allreduceResult[i] <<", got: "<<host[i]
//         <<"@["<<rank<<"]("<<i<<");"<<std::endl;
//     }
//   }
//   return 0;
// }

// template <>
// int verifyTransmit<sycl::half, BisectTransmit>(
//     sycl::half* host, sycl::half* host2,
//     uint32_t step, int rank, int world, uint32_t simd, size_t nelems
// ) {
//   /*
//   auto ret1 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage1Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage1Verify(
//       host, rank, step, nelems);
//   auto ret2 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage2Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage2Verify(
//       host, rank, step, nelems);
//   auto ret3 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage3Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage3Verify(
//       host, rank, step, nelems);
//       */
//   auto ret4 = (simd == 16) ?
//     bisectPAllReduce<sycl::half, 8, BisectTransmit, 16>::stage4Verify(
//       host2, rank, step, nelems) :
//     bisectPAllReduce<sycl::half, 8, BisectTransmit, 32>::stage4Verify(
//       host2, rank, step, nelems);
//   return /*ret1 + ret2 + ret3 + */ret4;
// }

// template <>
// int verifyTransmit<sycl::half, BisectPTransmit>(
//     sycl::half* host, sycl::half* host2,
//     uint32_t step, int rank, int world, uint32_t simd, size_t nelems
// ) {
//   auto ret4 = verifyAllReduce(host2, rank, world, nelems);
//   /*
//   auto ret1 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage1Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage1Verify(
//       host, rank, step, nelems);
//   auto ret2 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage2Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage2Verify(
//       host, rank, step, nelems);
//   auto ret3 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage3Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage3Verify(
//       host, rank, step, nelems);
//   auto ret4 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage4Verify(
//       host2, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage4Verify(
//       host2, rank, step, nelems);
//       */
//   return /*ret1 + ret2 + ret3 +*/ ret4;
// }

// template <>
// int verifyTransmit<sycl::half, BisectPPTransmit>(
//     sycl::half* host, sycl::half* host2,
//     uint32_t step, int rank, int world, uint32_t simd, size_t nelems
// ) {
//   /*
//   auto ret1 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage1Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage1Verify(
//       host, rank, step, nelems);
//   auto ret2 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage2Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage2Verify(
//       host, rank, step, nelems);
//   auto ret3 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage3Verify(
//       host, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage3Verify(
//       host, rank, step, nelems);
//       */
//   auto ret4 = (simd == 16) ?
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 16>::stage4Verify(
//       host2, rank, step, nelems) :
//     bisectAllReduce<sycl::half, 8, BisectTransmit, 32>::stage4Verify(
//       host2, rank, step, nelems);
//   return /*ret1 + ret2 + ret3 +*/ ret4;
// }

// template <typename T,
//          int NPeers,
//          template <typename, int, int> class Transmit,
//          int SubGroupSize>
// int AllReduce<T, NPeers, Transmit, SubGroupSize>::stage2Verify(
//     T* host, int rank, uint32_t flag, size_t nelems
// ){
//   constexpr auto n120B = 120 / sizeof(T);
//   constexpr auto n128B = 128 / sizeof(T);
//   constexpr auto wireCapInType = wireCapacity / sizeof(T);
//   constexpr auto wireTransInType = wireTransSize / sizeof(T);
//   constexpr auto NRanks = NPeers + 1;
//
//   auto nWorkElems = nelems / NRanks;
//   size_t nChunks = divUp(nWorkElems, wireCapInType);
//   auto nTransmitElems = nChunks * wireTransInType;
//
//   T* allRanks[NRanks];
//
//   for (int i = 0; i < NRanks; ++ i)
//     allRanks[i] = (T *)malloc(sizeof(T) * nWorkElems * NRanks);
//
//   __scope_guard free_pointers([&] {
//     for (int i = 0; i < NRanks; ++ i)
//       free(allRanks[i]);
//   });
//
//   for (int i = 0; i < NRanks; ++ i)
//     fill_pattern(allRanks[i], i, nelems);
//
//   // simulate an allreduce
//   allreduce(allRanks, NRanks, nelems);
//
//   // Check each gather buffer
//   for (int i = 0; i < NPeers; ++ i) {
//     int next = (rank + i + 1) % (NPeers + 1);
//     auto* peer_ptr = host + nTransmitElems * next;
//     auto* local_ptr = allRanks[0] + nWorkElems * next;
//
//     // we are expecting pattern = (scale | next)
//     for (int chunk = 0; chunk < nChunks; ++ chunk) {
//       T temp[n128B];
//       T scrat[n128B];
//
//       for (size_t b = 0, j = 0; b < wireCapInType; ++ b, ++ j) {
//         if (b + chunk * wireCapInType < nWorkElems) {
//           temp[b % n120B] = local_ptr[b + chunk * wireCapInType];
//         } else
//           temp[b % n120B] = -1.;
//         scrat[j % n128B] = peer_ptr[j + chunk * wireTransInType];
//
//         // wireCapInInt will be divided by n120B.
//         if (b % n120B == n120B -1) {
//           temp[60] = temp[30]; temp[61] = temp[31];
//
//           *(uint32_t *)&temp[30] = flag;
//           *(uint32_t *)&temp[62] = flag;
//
//           scrat[60] = peer_ptr[++j + chunk * wireTransInType];
//           scrat[61] = peer_ptr[++j + chunk * wireTransInType];
//
//           // flag
//           scrat[62] = peer_ptr[++j + chunk * wireTransInType];
//           scrat[63] = peer_ptr[++j + chunk * wireTransInType];
//
//           for (auto k = 0; k < n128B; ++ k) {
//             if (temp[k] - scrat[k] > 1e-5 && temp[k] != -1.) {
//               std::cout<<"["<<rank<<"] Verify failed @"
//                 <<i<<","<<k<<","<<b<<","<<chunk
//                 <<", expect:"<<temp[k]<<", but get:"<<scrat[k]<<std::endl;
//               return -1;
//     }}}}}
//   }
//
//   return 0;
// }

template<typename T>
int verifyTransmit(
    T* host, T* host2, uint32_t step, int rank, int world, uint32_t simd, size_t nelems
) {
  verifyAllReduce(host2, rank, world, nelems);
  return 0;
}

//
// We will remove sycl::event return in real API call.
// It's for test only.
//
template <typename T,
         template <typename, int> class Proto,
         template <typename, int, template <typename, int> class, int> class Transmit>
sycl::event testTransmit(
    sycl::nd_range<1> launchParam,
    T* input, T* ipcbuf0, T* ipcbuf1,
    T* const peerbuf0[], T* const peerbuf1[], size_t nelems,
    int rank, int world, uint32_t step, uint32_t subgroup, sycl::queue queue) {
  if (subgroup == 16) {
    constexpr int SubGroupSize = 16;
  switch(world) {
  case 2:
    return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
      sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          AllReduce<T, 2, Proto, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
            )
        );
    });
  case 4:
    return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
      sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          AllReduce<T, 4, Proto, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
            )
        );
    });
  case 8:
    return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
      sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          AllReduce<T, 8, Proto, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
            )
        );
    });
  default:
    throw std::logic_error("Unsupported communication topology");
  }
  } else {
    constexpr int SubGroupSize = 32;
    switch(world) {
    case 2:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          AllReduce<T, 2, Proto, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
      ));});
    case 4:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          AllReduce<T, 4, Proto, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
      ));});
    case 8:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          AllReduce<T, 8, Proto, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
        ));});
    default:
      throw std::logic_error("Unsupported communication topology");
    }
  }
}

// template <> sycl::event testTransmit <sycl::half, BisectTransmit> (
//     sycl::nd_range<1> launchParam,
//     sycl::half* input, sycl::half* ipcbuf0, sycl::half* ipcbuf1,
//     sycl::half* const peerbuf0[], sycl::half* const peerbuf1[], size_t nelems,
//     int rank, int world, uint32_t step, uint32_t subgroup, sycl::queue queue) {
//   if (subgroup == 16) {
//     constexpr int SubGroupSize = 16;
//     switch(world) {
//     case 8:
//       return queue.submit([&](sycl::handler &cgh) {
// #if defined(__enable_sycl_stream__)
//         sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
// #endif
//         cgh.parallel_for(
//           launchParam,
//           bisectAllReduce<sycl::half, 8, BisectTransmit, SubGroupSize>(
//             input, nelems, rank, step,
//             ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
// #if defined(__enable_sycl_stream__)
//             , cout
// #endif
//     ));});
//     default:
//       throw std::logic_error("Unsupported communication topology");
//     }
//   } else {
//     constexpr int SubGroupSize = 32;
//     switch(world) {
//     case 8:
//       return queue.submit([&](sycl::handler &cgh) {
// #if defined(__enable_sycl_stream__)
//         sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
// #endif
//         cgh.parallel_for(
//           launchParam,
//           bisectAllReduce<sycl::half, 8, BisectTransmit, SubGroupSize>(
//             input, nelems, rank, step,
//             ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
// #if defined(__enable_sycl_stream__)
//             , cout
// #endif
//     ));});
//     default:
//       throw std::logic_error("Unsupported communication topology");
//     }
//   }
// }

template <typename T, template <typename, int, int> class Transmit>
sycl::event testBisectTransmit (
    sycl::nd_range<1> launchParam,
    sycl::half* input, sycl::half* ipcbuf0, sycl::half* ipcbuf1,
    sycl::half* const peerbuf0[], sycl::half* const peerbuf1[], size_t nelems,
    int rank, int world, uint32_t step, uint32_t subgroup, sycl::queue queue) {
  if (subgroup == 16) {
    constexpr int SubGroupSize = 16;
    switch(world) {
    case 4:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          bisectPAllReduce<sycl::half, 4, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
      ));});
    case 8:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          bisectPAllReduce<sycl::half, 8, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
      ));});
    case 16:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          bisectPAllReduce<sycl::half, 16, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
      ));});
    default:
      throw std::logic_error("Unsupported communication topology");
    }
  } else {
    constexpr int SubGroupSize = 32;
    switch(world) {
    case 4:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          bisectPAllReduce<sycl::half, 4, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
      ));});
    case 8:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          bisectPAllReduce<sycl::half, 8, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
    ));});
    case 16:
      return queue.submit([&](sycl::handler &cgh) {
#if defined(__enable_sycl_stream__)
        sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
#endif
        cgh.parallel_for(
          launchParam,
          bisectPAllReduce<sycl::half, 16, Transmit, SubGroupSize>(
            input, nelems, rank, step,
            ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
#if defined(__enable_sycl_stream__)
            , cout
#endif
    ));});
    default:
      throw std::logic_error("Unsupported communication topology");
    }
  }
}

// template <> sycl::event testTransmit <sycl::half, BisectPPTransmit> (
//     sycl::nd_range<1> launchParam,
//     sycl::half* input, sycl::half* ipcbuf0, sycl::half* ipcbuf1,
//     sycl::half* const peerbuf0[], sycl::half* const peerbuf1[], size_t nelems,
//     int rank, int world, uint32_t step, uint32_t subgroup, sycl::queue queue) {
//   if (subgroup == 16) {
//     constexpr int SubGroupSize = 16;
// #if defined(__fix_param_passing__) && !defined(__enable_sycl_stream__)
//     bisectPPAllReduce<sycl::half, 8, BisectPPTransmit, SubGroupSize>
//       devOp(input, nelems, rank, step, ipcbuf0, ipcbuf1, peerbuf0, peerbuf1);
//     sycl::buffer params(
//       const_cast<const decltype(devOp) *>(&devOp),
//       sycl::range<1>(1)
//     );
// #endif
//     switch(world) {
//     case 8:
// #if defined(__fix_param_passing__) && !defined(__enable_sycl_stream__)
//       return queue.submit([&](sycl::handler &cgh) {
//         auto deviceCapture = params.template get_access<
//           sycl::access_mode::read, sycl::target::device>(cgh);
//         cgh.parallel_for(launchParam, [=] (sycl::nd_item<1> pos) {
//             deviceCapture[0](pos);
//         });
//       });
// #else
//       return queue.submit([&](sycl::handler &cgh) {
// #if defined(__enable_sycl_stream__)
//         sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
// #endif
//         cgh.parallel_for(
//           launchParam,
//           bisectPPAllReduce<sycl::half, 8, BisectPPTransmit, SubGroupSize>(
//             input, nelems, rank, step,
//             ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
// #if defined(__enable_sycl_stream__)
//             , cout
// #endif
//     ));});
// #endif
//     default:
//       throw std::logic_error("Unsupported communication topology");
//     }
//   } else {
//     constexpr int SubGroupSize = 32;
//     switch(world) {
//     case 8:
//       return queue.submit([&](sycl::handler &cgh) {
// #if defined(__enable_sycl_stream__)
//         sycl::stream cout(1024 * 1024, 16 * 1024, cgh);
// #endif
//         cgh.parallel_for(
//           launchParam,
//           bisectPPAllReduce<sycl::half, 8, BisectPPTransmit, SubGroupSize>(
//             input, nelems, rank, step,
//             ipcbuf0, ipcbuf1, peerbuf0, peerbuf1
// #if defined(__enable_sycl_stream__)
//             , cout
// #endif
//     ));});
//     default:
//       throw std::logic_error("Unsupported communication topology");
//     }
//   }
// }
//
template <typename T>
sycl::event testTransmit(
    std::string transmitType,
    sycl::nd_range<1> launchParam,
    T* input, T* ipcbuf0, T* ipcbuf1,
    T* const peerbuf0[], T* const peerbuf1[], size_t nelems,
    int rank, int world, uint32_t step, uint32_t subgroup, sycl::queue queue) {
  if (transmitType == "small") {
    return testTransmit<T, Rt64, ParallelTransmit>(
        launchParam,
        input, ipcbuf0, ipcbuf1, peerbuf0, peerbuf1,
        nelems, rank, world, step, subgroup, queue
    );
  } else if (transmitType == "simple") {
    return testTransmit<T, Rt64_128, ParallelTransmit>(
        launchParam,
        input, ipcbuf0, ipcbuf1, peerbuf0, peerbuf1,
        nelems, rank, world, step, subgroup, queue
    );
  } else if (transmitType == "bisect") {
    return testBisectTransmit<T, BisectPTransmit>(
        launchParam,
        input, ipcbuf0, ipcbuf1, peerbuf0, peerbuf1,
        nelems, rank, world, step, subgroup, queue
    );
  } else {
    throw std::logic_error("Transmit type not support");
  }
}

template sycl::event testTransmit<sycl::half, Rt64, ParallelTransmit>(
    sycl::nd_range<1> launchParam,
    sycl::half* input, sycl::half* ipcbuf0, sycl::half* ipcbuf1,
    sycl::half* const peerbuf0[], sycl::half* const peerbuf1[], size_t size,
    int rank, int world, uint32_t step, uint32_t simd, sycl::queue queue);

template sycl::event testTransmit<sycl::half, Rt64_128, ParallelTransmit>(
    sycl::nd_range<1> launchParam,
    sycl::half* input, sycl::half* ipcbuf0, sycl::half* ipcbuf1,
    sycl::half* const peerbuf0[], sycl::half* const peerbuf1[], size_t size,
    int rank, int world, uint32_t step, uint32_t simd, sycl::queue queue);

template sycl::event testBisectTransmit<sycl::half, BisectPTransmit>(
    sycl::nd_range<1> launchParam,
    sycl::half* input, sycl::half* ipcbuf0, sycl::half* ipcbuf1,
    sycl::half* const peerbuf0[], sycl::half* const peerbuf1[], size_t size,
    int rank, int world, uint32_t step, uint32_t simd, sycl::queue queue);

template int verifyTransmit<sycl::half>(
    sycl::half *, sycl::half *, uint32_t, int, int, uint32_t, size_t);

template sycl::event testTransmit<sycl::half>(
    std::string transmitType,
    sycl::nd_range<1> launchParam,
    sycl::half* input, sycl::half* ipcbuf0, sycl::half* ipcbuf1,
    sycl::half* const peerbuf0[], sycl::half* const peerbuf1[], size_t nelems,
    int rank, int world, uint32_t step, uint32_t subgroup, sycl::queue queue);
