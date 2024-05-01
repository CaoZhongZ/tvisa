CC=clang
CXX=clang++

OPT=-O3 -fno-strict-aliasing
# OPT=-g -fno-strict-aliasing
# VERBOSE=-D__enable_sycl_stream__

SYCLFLAGS=-fsycl -fsycl-targets=spir64_gen -Xsycl-target-backend=spir64_gen "-device pvc"

.PRECIOUS: %.o

# CCL_ROOT=../ccl/release/_install
# INCLUDES=-I$(CCL_ROOT)/include
# LIBRARIES=-L$(CCL_ROOT)/lib -lmpi -lze_loader

INCLUDES=-Itvisa/include
LIBRARIES=-lmpi -lze_loader

CXXFLAGS=-std=c++17 -fopenmp $(SYCLFLAGS) $(OPT) $(VERBOSE) -Wall -Wno-vla-cxx-extension $(INCLUDES) $(LIBRARIES)

main : ipc_exchange.cpp sycl_misc.cpp allreduce.cpp main.cpp

all : main

clean:
	rm -f main
