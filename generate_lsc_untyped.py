from enum import Enum

class DataShuffle(Enum):
    none = 0
    transpose = 1
    vnni = 2

class StoreShuffle(Enum):
    none = 0

class CacheCtrlNoLoadXe2(Enum):
    DEFAULT = 0
    L1UC_L3UC = 1
    L1UC_L3C = 2
    L1C_L3UC = 3
    L1C_L3C = 4
    L1S_L3UC = 5
    L1S_L3C = 6
    L1IAR_L3C = 7

class CacheCtrlNoStoreXe2(Enum):
    DEFAULT = 0
    L1UC_L3UC = 1
    L1UC_L3WB = 2
    L1WT_L3UC = 3
    L1WT_L3WB = 4
    L1S_L3UC = 5
    L1S_L3WB = 6
    L1WB_L3WB = 7

class CacheCtrlNoPrefetchXe2(Enum):
    DEFAULT = 0
    L1UC_L3C = 2
    L1C_L3UC = 3
    L1C_L3C = 4
    L1S_L3UC = 5
    L1S_L3C = 6
    L1IAR_L3C = 7

class CacheCtrlNoLoadCri(Enum):
    DEFAULT = 0
    L1UC_L2UC_L3UC = 2
    L1UC_L2UC_L3C = 3
    L1UC_L2C_L3UC = 4
    L1UC_L2C_L3C = 5
    L1C_L2UC_L3UC = 6
    L1C_L2UC_L3C = 7
    L1C_L2C_L3UC = 8
    L1C_L2C_L3C = 9
    L1S_L2UC_L3UC = 10
    L1S_L2UC_L3C = 11
    L1S_L2C_L3UC = 12
    L1S_L2C_L3C = 13
    L1RI_L2RI_L3RI = 14

class CacheCtrlNoStoreCri(Enum):
    DEFAULT = 0
    L1UC_L2UC_L3UC = 2
    L1UC_L2UC_L3WB = 3
    L1UC_L2WB_L3UC = 4
    L1UC_L2WB_L3WB = 5
    L1WT_L2UC_L3UC = 6
    L1WT_L2UC_L3WB = 7
    L1WT_L2WB_L3UC = 8
    L1WT_L2WB_L3WB = 9
    L1S_L2UC_L3UC = 10
    L1S_L2UC_L3WB = 11
    L1S_L2WB_L3UC = 12
    L1WB_L2UC_L3UC = 13
    L1WB_L2WB_L3UC = 14
    L1WB_L2UC_L3WB = 15

class CacheCtrlNoPrefetchCri(Enum):
    DEFAULT = 0
    L1UC_L2UC_L3C = 3
    L1UC_L2C_L3UC = 4
    L1UC_L2C_L3C = 5
    L1C_L2UC_L3UC = 6
    L1C_L2UC_L3C = 7
    L1C_L2C_L3UC = 8
    L1C_L2C_L3C = 9
    L1S_L2UC_L3UC = 10
    L1S_L2UC_L3C = 11
    L1S_L2C_L3UC = 12
    L1S_L2C_L3C = 13
    L1RI_L2RI_L3RI = 14

class CacheCtrlStrLoadXe2(Enum):
    DEFAULT = 'df.df'
    L1UC_L3UC = 'uc.uc'
    L1UC_L3C = 'uc.ca'
    L1C_L3UC = 'ca.uc'
    L1C_L3C = 'ca.ca'
    L1S_L3UC = 'st.uc'
    L1S_L3C = 'st.ca'
    L1IAR_L3C = 'ra.ca'

class CacheCtrlStrPrefetchXe2(Enum):
    DEFAULT = 'df.df'
    L1UC_L3UC = 'uc.uc'
    L1UC_L3C = 'uc.ca'
    L1C_L3UC = 'ca.uc'
    L1C_L3C = 'ca.ca'
    L1S_L3UC = 'st.uc'
    L1S_L3C = 'st.ca'
    L1IAR_L3C = 'ra.ca'

class CacheCtrlStrStoreXe2(Enum):
    DEFAULT = 'df.df'
    L1UC_L3UC = 'uc.uc'
    L1UC_L3WB = 'uc.wb'
    L1WT_L3UC = 'wt.uc'
    L1WT_L3WB = 'wt.wb'
    L1S_L3UC = 'st.uc'
    L1S_L3WB = 'st.wb'
    L1WB_L3WB = 'wb.wb'

class CacheCtrlStrLoadCri(Enum):
    DEFAULT = 'df.df.df'
    L1UC_L2UC_L3UC = 'uc.uc.uc'
    L1UC_L2UC_L3C = 'uc.uc.ca'
    L1UC_L2C_L3UC = 'uc.ca.uc'
    L1UC_L2C_L3C = 'uc.ca.ca'
    L1C_L2UC_L3UC = 'ca.uc.uc'
    L1C_L2UC_L3C = 'ca.uc.ca'
    L1C_L2C_L3UC = 'ca.ca.uc'
    L1C_L2C_L3C = 'ca.ca.ca'
    L1S_L2UC_L3UC = 'st.uc.uc'
    L1S_L2UC_L3C = 'st.uc.ca'
    L1S_L2C_L3UC = 'st.ca.uc'
    L1S_L2C_L3C = 'st.ca.ca'
    L1RI_L2RI_L3RI = 'ri.ri.ri'

class CacheCtrlStrPrefetchCri(Enum):
    DEFAULT = 'df.df.df'
    L1UC_L2UC_L3UC = 'uc.uc.uc'
    L1UC_L2UC_L3C = 'uc.uc.ca'
    L1UC_L2C_L3UC = 'uc.ca.uc'
    L1UC_L2C_L3C = 'uc.ca.ca'
    L1C_L2UC_L3UC = 'ca.uc.uc'
    L1C_L2UC_L3C = 'ca.uc.ca'
    L1C_L2C_L3UC = 'ca.ca.uc'
    L1C_L2C_L3C = 'ca.ca.ca'
    L1S_L2UC_L3UC = 'st.uc.uc'
    L1S_L2UC_L3C = 'st.uc.ca'
    L1S_L2C_L3UC = 'st.ca.uc'
    L1S_L2C_L3C = 'st.ca.ca'
    L1RI_L2RI_L3RI = 'ri.ri.ri'

class CacheCtrlStrStoreCri(Enum):
    DEFAULT = 'df.df.df'
    L1UC_L2UC_L3UC = 'uc.uc.uc'
    L1UC_L2UC_L3WB = 'uc.uc.wb'
    L1UC_L2WB_L3UC = 'uc.wb.uc'
    L1UC_L2WB_L3WB = 'uc.wb.wb'
    L1WT_L2UC_L3UC = 'wt.uc.uc'
    L1WT_L2UC_L3WB = 'wt.uc.wb'
    L1WT_L2WB_L3UC = 'wt.wb.uc'
    L1WT_L2WB_L3WB = 'wt.wb.wb'
    L1S_L2UC_L3UC = 'st.uc.uc'
    L1S_L2UC_L3WB = 'st.uc.wb'
    L1S_L2WB_L3UC = 'st.wb.uc'
    L1WB_L2UC_L3UC = 'wb.uc.uc'
    L1WB_L2WB_L3UC = 'wb.wb.uc'
    L1WB_L2UC_L3WB = 'wb.uc.wb'

def generate_ecode(op, data_size, des_len, shuf, cache, is_cri):
    base = 0
    base = base | op

    if is_cri:
        if shuf.name == 'vnni':
            base = base | (1 << 9)
        elif shuf.name == 'transpose':
            base = base | (1 << 10)

        base = base | (data_size << 11)
        base = base | (2 << 14)
        base = base | (cache.value << 16)
    else:
        if shuf.name == 'vnni':
            base = base | (1 << 7)
        elif shuf.name == 'transpose':
            base = base | (1 << 15)

        base = base | (data_size << 9)
        base = base | (cache.value << 17)
        if des_len == 32:
            des_len = 31
        base = base | (des_len << 20)
        base = base | (1 << 25)
    return hex(base)

def generate_rawsends(file_name, op):
    def encode_rawsends(
        op, shuf, data_sizes, des_lens, caches, is_cri, file
    ):
        for data_size in data_sizes:
            for des_len in des_lens:
                for cache in caches:
                    encode = generate_ecode(
                        op, data_size, des_len, shuf, cache, is_cri
                    )
                    func_str = f"{func_name}({data_size}, {des_len}, DataShuffle::{shuf.name}, CacheCtrl::{cache.name}, {encode});\n"
                    file.write(func_str)

    if op == 3:
        des_lens = range(1, 33)
        func_name = 'EnumerateLoads'
        shuffles = DataShuffle
        cachectrl_xe2 = CacheCtrlNoLoadXe2
        cachectrl_cri = CacheCtrlNoLoadCri
    elif op == 7:
        des_lens = range(1, 17)
        func_name = 'EnumerateStores'
        shuffles = StoreShuffle
        cachectrl_xe2 = CacheCtrlNoStoreXe2
        cachectrl_cri = CacheCtrlNoStoreCri
    elif op == 2:
        op = 3
        des_lens = [0]
        func_name = 'EnumeratePrefetch'
        shuffles = DataShuffle
        cachectrl_xe2 = CacheCtrlNoPrefetchXe2
        cachectrl_cri = CacheCtrlNoPrefetchCri

    def generate_entries(file, cachectrl, is_cri):
        for shuf in shuffles:
            if shuf.value == 1:
                data_sizes = [2, 3]
            elif shuf.value == 2:
                data_sizes = [0, 1]
            else:
                data_sizes = [0, 1, 2, 3]
            encode_rawsends(
                op, shuf, data_sizes, des_lens, cachectrl, is_cri, file
            )

    with open(file_name, 'w') as file:
        file.write("#if defined(__SYCL_TARGET_INTEL_GPU_CRI__)\n")
        generate_entries(file, cachectrl_cri, True)
        file.write("#else\n")
        generate_entries(file, cachectrl_xe2, False)
        file.write("#endif\n")

class DataWidth(Enum):
    d8c32 = 1
    d16c32 = 2
    d32 = 4
    d64 = 8

def generate_lsc_untyped(filename, op):
    vecmap = {1:'', 2:'x2', 4:'x4', 8:'x8'}
    datamap = {1 :'d8c32', 2:'d16c32', 4:'d32', 8:'d64'}
    if op == 'EnumerateLSCLoad':
        cachectrl_xe2 = CacheCtrlStrLoadXe2
        cachectrl_cri = CacheCtrlStrLoadCri
    elif op == 'EnumerateLSCStore':
        cachectrl_xe2 = CacheCtrlStrStoreXe2
        cachectrl_cri = CacheCtrlStrStoreCri
    elif op == 'EnumerateLSCPrefetch':
        cachectrl_xe2 = CacheCtrlStrPrefetchXe2
        cachectrl_cri = CacheCtrlStrPrefetchCri

    def generate_entries(file, cachectrl):
        for data_size in [1,2,4,8]:
            for vec_size in [1,2,4,8] if data_size < 8 else [1, 2]:
                actual_size = data_size * vec_size
                actual_vec = 1

                if actual_size > 4 and data_size != 8:
                    actual_vec = actual_size // 4
                    actual_size = 4
                elif actual_size > 8:
                    actual_vec = actual_size // 8
                    actual_size = 8

                for sg_sz in [8, 16, 32]:
                    for cache in cachectrl:
                        func_str = f"{op}({data_size}, {vec_size}, {sg_sz}, CacheCtrl::{cache.name}, {cache.value}, {datamap[actual_size]}{vecmap[actual_vec]});\n"
                        file.write(func_str)
                        # print(func_str)

    with open(filename, 'w') as file:
        file.write("#if defined(__SYCL_TARGET_INTEL_GPU_CRI__)\n")
        generate_entries(file, cachectrl_cri)
        file.write("#else\n")
        generate_entries(file, cachectrl_xe2)
        file.write("#endif\n")

if __name__ == "__main__":
    generate_rawsends('./include/list_raw_prefetches.list', 2)
    generate_rawsends('./include/list_raw_loads.list', 3)
    generate_rawsends('./include/list_raw_stores.list', 7)
    generate_lsc_untyped('./include/list_ugm_prefetches.list', 'EnumerateLSCPrefetch')
    generate_lsc_untyped('./include/list_ugm_loads.list', 'EnumerateLSCLoad')
    generate_lsc_untyped('./include/list_ugm_stores.list', 'EnumerateLSCStore')
