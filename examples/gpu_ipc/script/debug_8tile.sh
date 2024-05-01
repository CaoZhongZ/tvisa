#!/bin/bash
export FI_PROVIDER=tcp
export ONEAPI_DEVICE_SELECTOR=level_zero:gpu
# export PATH=/home/caozhong/Workspace/ccl/release/_install/bin:$PATH
# export LD_LIBRARY_PATH=/home/caozhong/Workspace/ccl/release/_install/lib:$LD_LIBRARY_PATH

mpirun -disable-auto-cleanup \
  -np 2 ./main $@ : \
  -np 1 ./main $@ : \
  -np 1 ./main $@ : \
  -np 1 gdbserver :44555 ./main $@ : \
  -np 3 ./main $@
