#include "lsc.hpp"
#include "regmap.hpp"

// #define __SYCL_DEVICE_ONLY__

template <typename T, int TileHeight, int TileWidth, DataShuffle Transpose,
          int SubGroupSize = 16>
struct TileLoad;

template <typename T, int TileHeight, int TileWidth, DataShuffle Transpose,
          int SubGroupSize = 16>
struct TileStore;

#if defined(__SYCL_DEVICE_ONLY__)
template <typename T, int TileHeight, int TileWidth, int SubGroupSize>
struct TileLoad<T, TileHeight, TileWidth, DataShuffle::none, SubGroupSize> {
  static constexpr int N = InnerLayout<T, TileHeight, TileWidth,
                                       DataShuffle::none, SubGroupSize>::N;
  // TODO: no necessary to load maximum for every time
  static constexpr int max_load_height_in_elem = 32;
  static constexpr int max_load_width_in_bytes = 32;
  static constexpr int default_load_height_in_elem = 8;
  static constexpr int default_load_width_in_bytes = 32;
  static constexpr int load_height_in_elem =
      max_load_height_in_elem > TileHeight ? TileHeight
                                           : max_load_height_in_elem;
  static constexpr int load_width_in_bytes = max_load_width_in_bytes >
                                                     TileWidth * sizeof(T)
                                                 ? TileWidth * sizeof(T)
                                                 : max_load_width_in_bytes;
  static constexpr int load_width_in_elem = load_width_in_bytes / sizeof(T);                                            

  static_assert(TileWidth % load_width_in_elem == 0,
                "tile width must be a multiple of load_width_in_bytes");
  static_assert(TileHeight % load_height_in_elem == 0,
                "tile height must be a multiple of load_height_in_elem");
  static constexpr int num_block_h = TileHeight / load_height_in_elem;
  static constexpr int num_block_w =
      TileWidth / load_width_in_elem;
  using matrix_type = __ArrayMatrix<T, load_height_in_elem, load_width_in_elem,
                                    DataShuffle::none, SubGroupSize>;
  using storage_type = typename matrix_type::storage_type;                                    
  using address_payload_type =
      AddressPayload<load_height_in_elem, load_width_in_elem>;
  static constexpr int num_elem = N / matrix_type::N;
  static_assert(num_elem == num_block_h * num_block_w, "load with tail is not supported currently");
  inline void operator()(T *SurfaceBase, uint32_t SurfaceHeight, uint32_t SurfaceWidth,
                  uint32_t SurfacePitch, int Src0AddrX, int Src0AddrY,
                  storage_type (& tile)[num_elem]) {
    address_payload_type address_payload(SurfaceBase, SurfaceHeight,
                                         SurfaceWidth, SurfacePitch, Src0AddrX,
                                         Src0AddrY);

    for (int i = 0; i < num_block_h; ++i) {
      address_payload.UpdateSrc0AddrY(Src0AddrY + i * load_height_in_elem);
      for (int j = 0; j < num_block_w; ++j) {
        // TODO: fix,
        // 1. update payload,
        // 2. remove copy from subtile to tile
        address_payload.UpdateSrc0AddrX(Src0AddrX + j * load_width_in_bytes);
        matrix_type sub_tile;
        lscLoad(sub_tile, address_payload);
        
        tile[i * num_block_w + j] = std::move(sub_tile.getStorage());
      }
    }
  }
};

template <typename T, int TileHeight, int TileWidth, int SubGroupSize>
struct TileStore<T, TileHeight, TileWidth, DataShuffle::none, SubGroupSize> {
  static constexpr int N = InnerLayout<T, TileHeight, TileWidth,
                                       DataShuffle::none, SubGroupSize>::N;
  // TODO: 
  static constexpr int max_store_height_in_elem = 8;
  static constexpr int max_store_width_in_bytes = 32;
  static constexpr int store_height_in_elem = 8;
  static constexpr int store_width_in_bytes = 32;
  static constexpr int store_width_in_elem = store_width_in_bytes / sizeof(T);
  static_assert(TileWidth * sizeof(T) % store_width_in_bytes == 0,
                "tile width must be a multiple of store_width_in_bytes");
  static_assert(TileHeight % store_height_in_elem == 0,
                "tile height must be a multiple of store_height_in_elem");
  static constexpr int num_block_h = TileHeight / store_height_in_elem;
  static constexpr int num_block_w =
      TileWidth * sizeof(T) / store_width_in_bytes;
  using matrix_type =
      __ArrayMatrix<T, store_height_in_elem, store_width_in_elem,
                    DataShuffle::none, SubGroupSize>;
  using storage_type = typename matrix_type::storage_type;                         
  using address_payload_type =
      AddressPayload<store_height_in_elem, store_width_in_elem>;
  static constexpr int num_elem = N / matrix_type::N;
  static_assert(num_elem == num_block_h * num_block_w, "store with tile is not supported currently");  
  inline void operator()(T *SurfaceBase, uint32_t SurfaceHeight, uint32_t SurfaceWidth,
                  uint32_t SurfacePitch, int Src0AddrX, int Src0AddrY,
                 storage_type (& tile)[num_elem]) {
    address_payload_type address_payload(SurfaceBase, SurfaceHeight,
                                         SurfaceWidth, SurfacePitch, Src0AddrX,
                                         Src0AddrY);
    for (int i = 0; i < num_block_h; ++i) {
      address_payload.UpdateSrc0AddrY(Src0AddrY + i * store_height_in_elem);
      for (int j = 0; j < num_block_w; ++j) {
        // TODO: fix,
        // 1. update payload, not init every time
        // 2. remove copy from subtile to tile
        address_payload.UpdateSrc0AddrX(Src0AddrX + j * store_width_in_bytes);
        matrix_type sub_tile(std::move(tile[i * num_block_w + j]));
        lscStore(address_payload, sub_tile);
      }
    }
  }
};
#endif
