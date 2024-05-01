import torch.nn.parallel
import torch.distributed as dist
import intel_extension_for_pytorch as ipex
import oneccl_bindings_for_pytorch
import os
import time

# local
os.environ['LOCAL_RANK'] = os.environ['MPI_LOCALRANKID']
os.environ['LOCAL_WORLD_SIZE'] = os.environ['MPI_LOCALNRANKS']
# global
os.environ['RANK'] = os.environ['PMI_RANK']
os.environ['WORLD_SIZE'] = os.environ['PMI_SIZE']
# deepspeed multi-node
os.environ['CROSS_RANK'] = '0'
os.environ['CROSS_SIZE'] = '1'
#
os.environ['MASTER_ADDR'] = 'localhost'
os.environ['MASTER_PORT'] = '12121'

dist.init_process_group('ccl')

rank = dist.get_rank()
size = dist.get_world_size()

# x0 = torch.arange(0, 14336, device=torch.device('xpu', rank + 2), dtype = torch.float);
# x1 = torch.arange(1, 14337, device = torch.device('xpu', rank + 2), dtype = torch.float);
# x = torch.vstack([x0, x1])
x = torch.ones([4, 1, 14336], device=torch.device('xpu', rank), dtype=torch.float16)
# x = x / 1024

y = torch.ones([4, 1, 14336], device=torch.device('xpu', rank), dtype=torch.float16)

print("start:")

# dist.all_reduce(y)
print(y)

dist.all_reduce(y)
# dist.all_reduce(y)

print("result:", y)

x = x + 1
dist.all_reduce(x)
print("second:", x)

dist.destroy_process_group()
