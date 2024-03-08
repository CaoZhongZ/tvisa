#!/bin/bash
set -e

echo "Subgroup weakscaling test"
./gemm -M 32 -K 16 -N 64 -m 1 -n 1 -l 1 -w 1

echo "-----------------horizontal explore-------------------"
./gemm -M 32 -K 32 -N 128 -m 1 -n 1 -l 1 -w 2
./gemm -M 32 -K 32 -N 192 -m 1 -n 1 -l 1 -w 3
./gemm -M 32 -K 32 -N 256 -m 1 -n 1 -l 1 -w 4

echo "-----------------vertical explore---------------------"
./gemm -M 64 -K 16 -N 64 -m 1 -n 1 -l 2 -w 1
./gemm -M 96 -K 16 -N 64 -m 1 -n 1 -l 3 -w 1
./gemm -M 128 -K 16 -N 64 -m 1 -n 1 -l 4 -w 1
