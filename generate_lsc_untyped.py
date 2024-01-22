from enum import Enum

class DataShuffle(Enum):
    none = 0
    transpose = 1
    vnni = 2

class CacheString(Enum):
    DEFAULT = 0
    L1UC_L3UC = 1
    L1UC_L3WB = 2
    L1WT_L3UC = 3
    L1WT_L3WB = 4
    L1S_L3UC = 5
    L1S_L3WB = 6
    L1WB_L3WB = 7

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

def generate_functions(file_name, op):
    data_sizes = [0, 1, 2, 3]
    if op == 3:
        des_lens = range(1, 33)
        func_name = 'EnumerateLoads'
    elif op == 7:
        des_lens = range(1, 9)
        func_name = 'EnumerateStores'

    with open(file_name, 'w') as file:
        for data_size in data_sizes:
            for des_len in des_lens:
                for shuf in DataShuffle:
                    for cache in CacheString:
                        encode = generate_ecode(op, data_size, des_len, shuf, cache)
                        func_str = f"{func_name}({data_size}, {des_len}, DataShuffle::{shuf.name}, CacheCtrl::{cache.name}, {encode});\n"
                        file.write(func_str)
                        # print(func_str)

if __name__ == "__main__":
    shuf = DataShuffle.none
    cache = CacheString.DEFAULT

    op = 3
    data_size = 2
    den_len = 8
    # base = generate_ecode(op, data_size, den_len, shuf, cache)
    # print(hex(base))
    generate_functions('./include/list_raw_loads.list', 3)
    generate_functions('./include/list_raw_stores.list', 7)

