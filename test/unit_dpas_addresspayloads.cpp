#include <iostream>
#include <sycl/sycl.hpp>

#include "cxxopts.hpp"
#include "sycl_misc.hpp"
#include "gen_visa_templates.hpp"
#include <mkl.h>
#include <cfloat>
#include <cmath>

#define SG_SZ 16

size_t parse_nelems(const std::string& nelems_string) {
  size_t base = 1;
  size_t pos = nelems_string.rfind("K");
  if (pos != std::string::npos) {
    base = 1024ull;
  } else {
    pos = nelems_string.rfind("M");
    if (pos != std::string::npos)
      base = 1024 * 1024ull;
    else {
      pos = nelems_string.rfind("G");
      if (pos != std::string::npos)
        base = 1024 * 1024 * 1024ull;
    }
  }

  return stoull(nelems_string) * base;
}

template <typename T>
void fill_sequential(T *p, int rank, size_t nelems) {
  for (size_t i = 0; i < nelems; ++ i) {
    p[i] = i % 3;
  }
}

template <typename T>
void show_tile(T* start, int W, int H, size_t pitch) {
  auto* tile = reinterpret_cast<T (*)[pitch/sizeof(T)]>(start);
  for (int h = 0; h < H; ++ h) {
    for (int w = 0; w < W; ++ w) {
      std::cout<<tile[h][w]<<", ";
    }
    std::cout<<std::endl;
  }
}


template<typename T1,typename T2>
bool all_close(
    const T1 *actual, int lda, const T2 *desired, int ldb, const int M, const int N, const float rtol=1e-3, const float atol=1e-3) {
  std::pair<int, int> maximum_idx;
  float maximum_err = 0.f;
  float tol = 0.f;
  for (int m = 0; m < M; ++m) {
    for (int n = 0; n < N; ++n) {
      const float a = static_cast<float>(actual[m * lda + n]);
      const float b = static_cast<float>(desired[m * ldb + n]);      
      const float err = std::fabs(a - b);
      if (err > maximum_err) {
        printf("Error! Matrix[%d, %d]=%.8f, ref=%.8f, error = %.8f\n", m, n, a, b, err);
        fflush(stdout);
        maximum_idx = {m, n};
        maximum_err = err;
        tol = atol + rtol * std::fabs(b);
      }
    }
  }
  if (maximum_err > tol) {
    return false;
  }
  return true;
}

template <typename T>
void verify_result(const T* actual_result, const T* srcA, const T* srcB, int M, int K, int N, int lda, int ldb, int ldc){
  std::vector<float> a(M * K), b(K * N);
  for(int i=0; i<M; ++i){
    for(int j=0; j<K; ++j) {
      a[i * K + j] = static_cast<float>(srcA[i * lda + j]);
    }
  }
  for(int i=0; i<K; ++i){
    for(int j=0; j<N; ++j) {
      b[i * N + j] = static_cast<float>(srcB[i * ldb + j]);
    }
  }  
  
  std::vector<float> expected(M * N, 0);
  
  // cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1.0f, a.data(), K, b.data(), N, 0, expected.data(), N);
  
  bool res = all_close(actual_result, ldc, expected.data(), N, M, N);
  if (res) 
    printf("test passed\n");
  else
    printf("test failed\n");  
}


template <typename T, int M, int K, int N, typename Enable = void>
struct tile_accumulate;

template <typename T, int M, int K, int N>
struct tile_accumulate<T, M, K, N, typename std::enable_if_t<K == 16 && N == 16, void>> {
  tile_accumulate(T* dst, T* src, int surfaceW, int surfaceH, int surfaceP)
    : src(src), dst(dst), surfaceH(surfaceH), surfaceW(surfaceW), surfaceP(surfaceP)
  {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::id<1> index) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto grp_num = index / SG_SZ;

    auto x_off = grp_num * 16;
    auto y_off = 0;

    AddressPayload<M, K> srcAAddress(src,
        surfaceW, surfaceH, surfaceP, x_off, y_off);
    AddressPayload<K, N> srcBAddress(src,
        surfaceW, surfaceH, surfaceP, x_off, y_off);        

    AddressPayload<M, N> dstAddress(srcAAddress);
    dstAddress.updateSurfaceBase(dst);

    __ArrayMatrix<T, M, K, DataShuffle::none, 16> tmp0;
    __ArrayMatrix<T, K, N, DataShuffle::vnni, 16> tmp1;

    lscLoad(tmp0, srcAAddress);
    lscLoad(tmp1, srcBAddress);

    __ArrayMatrix<T, M, N, DataShuffle::none, 16> acc;
    acc.zero();

    swFence();
    dpas(acc, tmp0, tmp1);
    swFence();
    
    lscStore(dstAddress, acc);
#else
    dst[index] += src[index];
#endif
  }

private:
  T* src;
  T* dst;
  int surfaceH;
  int surfaceW;
  int surfaceP;
};


template <typename T, int M, int K, int N>
struct tile_accumulate<T, M, K, N, typename std::enable_if_t<K == 16 && (N > 16) && (N % 16 == 0), void>> {
  tile_accumulate(T* dst, T* src, int surfaceW, int surfaceH, int surfaceP)
    : src(src), dst(dst), surfaceH(surfaceH), surfaceW(surfaceW), surfaceP(surfaceP)
  {}

  void operator() [[sycl::reqd_sub_group_size(SG_SZ)]] (sycl::id<1> index) const {
#if defined(__SYCL_DEVICE_ONLY__)
    auto grp_num = index / SG_SZ;

    auto x_off = grp_num * 16;
    auto y_off = 0;

    // load a
    AddressPayload<M, 16> srcAAddress(src,
        surfaceW, surfaceH, surfaceP, x_off, y_off);
    __ArrayMatrix<T, M, 16, DataShuffle::none, 16> tmp0;
    lscLoad(tmp0, srcAAddress);
    
    // load b
    AddressPayload<16, 16> srcBAddress(src,
        surfaceW, surfaceH, surfaceP, x_off, y_off);        
    __ArrayMatrix<T, 16, 16, DataShuffle::vnni, 16> tmp1[N/16];
    constexpr int N_Loop = N / 16;
    #pragma unroll
    for(int i=0; i<N_Loop; ++i) {
      srcBAddress.UpdateSrc0AddrX(y_off + i * 16);
      lscLoad(tmp1[i], srcBAddress);
    }
    
    // MMA
    __ArrayMatrix<T, M, 16, DataShuffle::none, 16> acc[N/16];    

    #pragma unroll
    for(int i=0; i<N_Loop; ++i) {
      acc[i].zero();
    }

    swFence();

    #pragma unroll
    for(int i=0; i<N_Loop; ++i) {
      dpas(acc[i], tmp0, tmp1[i]);            
    }

    swFence();
    
    // store c
    AddressPayload<M, 16> dstAddress(srcAAddress);
    dstAddress.updateSurfaceBase(dst);
    #pragma unroll
    for(int i=0; i<N_Loop; ++i) {
      dstAddress.UpdateSrc0AddrX(y_off + i * 16);             
      lscStore(dstAddress, acc[i]);  
    }
#else
    dst[index] += src[index];
#endif
  }

private:
  T* src;
  T* dst;
  int surfaceH;
  int surfaceW;
  int surfaceP;
};

int main(int argc, char *argv[]) {
  cxxopts::Options opts("Copy", "Copy baseline for performance");
  opts.allow_unrecognised_options();
  opts.add_options()
    ("p,pitch", "Pitch of the surface",
     cxxopts::value<size_t>()->default_value("4096"))
    ("s,surround", "Height of the outer surface",
     cxxopts::value<size_t>()->default_value("4096"))
    ("w,width", "Width of the surface",
     cxxopts::value<int>()->default_value("1024"))
    ("h,height", "Height of the surface",
     cxxopts::value<int>()->default_value("1024"))
    ("g,groups", "Number of subgroups",
     cxxopts::value<size_t>()->default_value("1"))
    ;

  auto parsed_opts = opts.parse(argc, argv);

  auto surfaceP = parsed_opts["pitch"].as<size_t>();
  auto surround = parsed_opts["surround"].as<size_t>();
  auto surfaceH = parsed_opts["height"].as<int>();
  auto surfaceW = parsed_opts["width"].as<int>();

  auto groups = parsed_opts["groups"].as<size_t>();

  using t_type = sycl::half;

  auto nelems = surfaceP * surround / sizeof(t_type);
  auto alloc_size = surfaceP * surround;
  
  sycl::queue queue = currentQueue(0, 0);

  auto device = queue.get_device();
  std::cout << "Device: " << device.get_info<sycl::info::device::name>()
                << ", Platform: "
                << device.get_platform().get_info<sycl::info::platform::name>() << std::endl;

  auto* src = sycl::malloc_device(alloc_size, queue);
  auto* dst = sycl::malloc_device(alloc_size, queue);
  auto* b_host = sycl::malloc_host(alloc_size, queue);
  auto* b_check = sycl::malloc_host(alloc_size, queue);

  release_guard __guard([&]{
    sycl::free(src, queue);
    sycl::free(dst, queue);
    sycl::free(b_host, queue);
    sycl::free(b_check, queue);
  });

  fill_sequential(
      (t_type *)b_host, 0., nelems
  );

  queue.memcpy(src, b_host, alloc_size);
  queue.wait();

  constexpr int M = 32;
  constexpr int K = 16;
  constexpr int N = 64;

  queue.fill<t_type>(dst, 8., nelems);

  queue.submit([&](sycl::handler &h) {
    h.parallel_for(sycl::range<1> { groups * SG_SZ },
        tile_accumulate<t_type, M, K, N>(
          reinterpret_cast<t_type *>(dst),
          reinterpret_cast<t_type *>(src),
          surfaceW, surfaceH, surfaceP));
  });

  queue.memcpy(b_check, dst, alloc_size);
  queue.wait();


  std::cout<<"------------------------------------------------"<<std::endl;
  int ld = surfaceP / sizeof(t_type);
  verify_result((t_type *)b_check, (t_type *)b_host, (t_type *)b_host, M, K, N, 
    ld, ld, ld);
}
