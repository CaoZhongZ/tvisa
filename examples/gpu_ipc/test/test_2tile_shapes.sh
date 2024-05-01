#!/bin/bash
set -e

shapes="128 256 384 1280 1920 1024 2048 14336 16384"
groups="1 2 4 8 16 32 64 7 13 15 17 19"
subgroups="1 2 4 8 32 64"

echo "Start testing"

for shape in $shapes
do
  for group in $groups
  do
    for subgroup in $subgroups
    do
      echo "Run $shape $group $subgroup"
      . ./script/run_2tile.sh -v -n $shape -g$group -w$subgroup || exit 1
    done
  done
done
