from enum import Enum

class DataShuffle(Enum):
    none = 0
    transpose = 1
    vnni = 2

class StoreShuffle(Enum):
    none = 0

class CacheCtrlNoLoad(Enum):
    DEFAULT = 0
    L1UC_L3UC = 1
    L1UC_L3C = 2
    L1C_L3UC = 3
    L1C_L3C = 4
    L1S_L3UC = 5
    L1S_L3C = 6
    L1IAR_L3C = 7

class CacheCtrlNoStore(Enum):
    DEFAULT = 0
    L1UC_L3UC = 1
    L1UC_L3WB = 2
    L1WT_L3UC = 3
    L1WT_L3WB = 4
    L1S_L3UC = 5
    L1S_L3WB = 6
    L1WB_L3WB = 7

class CacheCtrlNoPrefetch(Enum):
    DEFAULT = 0
    L1UC_L3C = 2
    L1C_L3UC = 3
    L1C_L3C = 4
    L1S_L3UC = 5
    L1S_L3C = 6
    L1IAR_L3C = 7

class CacheCtrlStrLoad(Enum):
    DEFAULT = 'df.df'
    L1UC_L3UC = 'uc.uc'
    L1UC_L3C = 'uc.ca'
    L1C_L3UC = 'ca.uc'
    L1C_L3C = 'ca.ca'
    L1S_L3UC = 'st.uc'
    L1S_L3C = 'st.ca'
    L1IAR_L3C = 'ra.ca'

class CacheCtrlStrPrefetch(Enum):
    DEFAULT = 'df.df'
    L1UC_L3UC = 'uc.uc'
    L1UC_L3C = 'uc.ca'
    L1C_L3UC = 'ca.uc'
    L1C_L3C = 'ca.ca'
    L1S_L3UC = 'st.uc'
    L1S_L3C = 'st.ca'
    L1IAR_L3C = 'ra.ca'

class CacheCtrlStrStore(Enum):
    DEFAULT = 'df.df'
    L1UC_L3UC = 'uc.uc'
    L1UC_L3WB = 'uc.wb'
    L1WT_L3UC = 'wt.uc'
    L1WT_L3WB = 'wt.wb'
    L1S_L3UC = 'st.uc'
    L1S_L3WB = 'st.wb'
    L1WB_L3WB = 'wb.wb'

def generate_ecode(op, data_size, des_len, shuf, cache):
    base = 0
    base = base | op

    if (shuf.name == 'vnni'):
        base = base | (1 << 7)
    elif (shuf.name == 'transpose'):
        base = base | (1 << 15)

    base = base | (data_size << 9)
    base = base | (cache.value << 17)
    base = base | (des_len << 20)
    base = base | (1 << 25)
    return hex(base)

def generate_rawsends(file_name, op):
    def encode_rawsends(op, shuf, data_sizes, des_lens, caches, file):
        for data_size in data_sizes:
            for des_len in des_lens:
                for cache in caches:
                    encode = generate_ecode(op, data_size, des_len, shuf, cache)
                    func_str = f"{func_name}({data_size}, {des_len}, DataShuffle::{shuf.name}, CacheCtrl::{cache.name}, {encode});\n"
                    file.write(func_str)

    if op == 3:
        des_lens = range(1, 33)
        func_name = 'EnumerateLoads'
        shuffles = DataShuffle
        cachectrl = CacheCtrlNoLoad
    elif op == 7:
        des_lens = range(1, 9)
        func_name = 'EnumerateStores'
        shuffles = StoreShuffle
        cachectrl = CacheCtrlNoStore
    elif op == 2:
        op = 3
        des_lens = [0]
        func_name = 'EnumeratePrefetch'
        shuffles = DataShuffle
        cachectrl = CacheCtrlNoPrefetch

    with open(file_name, 'w') as file:
        for shuf in shuffles:
            if shuf.value == 1:
                data_sizes = [2, 3]
            elif shuf.value == 2:
                data_sizes = [0, 1]
            else:
                data_sizes = [0, 1, 2, 3]
            encode_rawsends(op, shuf, data_sizes, des_lens, cachectrl, file)

class DataWidth(Enum):
    d8c32 = 1
    d16c32 = 2
    d32 = 4
    d64 = 8

def generate_lsc_untyped(filename, op):
    vecmap = {1:'', 2:'x2', 4:'x4', 8:'x8'}
    datamap = {1 :'d8c32', 2:'d16c32', 4:'d32', 8:'d64'}
    if op == 'EnumerateLSCLoad':
        cachectrl = CacheCtrlStrLoad
    elif op == 'EnumerateLSCStore':
        cachectrl = CacheCtrlStrStore
    elif op == 'EnumerateLSCPrefetch':
        cachectrl = CacheCtrlStrPrefetch

    with open(filename, 'w') as file:
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

                for sg_sz in [16, 32]:
                    for cache in cachectrl:
                        func_str = f"{op}({data_size}, {vec_size}, {sg_sz}, CacheCtrl::{cache.name}, {cache.value}, {datamap[actual_size]}{vecmap[actual_vec]});\n"
                        file.write(func_str)
                        # print(func_str)

if __name__ == "__main__":
    generate_rawsends('./include/list_raw_prefetches.list', 2)
    generate_rawsends('./include/list_raw_loads.list', 3)
    generate_rawsends('./include/list_raw_stores.list', 7)
    generate_lsc_untyped('./include/list_ugm_prefetches.list', 'EnumerateLSCPrefetch')
    generate_lsc_untyped('./include/list_ugm_loads.list', 'EnumerateLSCLoad')
    generate_lsc_untyped('./include/list_ugm_stores.list', 'EnumerateLSCStore')

