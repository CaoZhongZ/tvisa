#!/bin/bash
export FI_PROVIDER=tcp
export ONEAPI_DEVICE_SELECTOR=level_zero:gpu
# export PATH=/home/caozhong/Workspace/ccl/release/_install/bin:$PATH
# export LD_LIBRARY_PATH=/home/caozhong/Workspace/ccl/release/_install/lib:$LD_LIBRARY_PATH

# gdbserver1="gdbserver :44444"
# gdbserver2="gdbserver :44555"
gdbserver3="gdbserver :44666"
# gdbserver4="gdbserver :44777"

mpirun -disable-auto-cleanup \
  -np 1 $gdbserver1 ./main $@ : \
  -np 1 $gdbserver2 ./main $@ : \
  -np 1 $gdbserver3 ./main $@ : \
  -np 1 $gdbserver4 ./main $@
