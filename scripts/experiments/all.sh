#!/bin/bash

cwd=$(pwd)
source ./bootstrap.sh

echo "#0 Running"
source $cwd/kernelization-kagen-rhg.sh

echo "#1 Running"
source $cwd/kernelization-kagen.sh

echo "#2 Running"
source $cwd/linear-kernel-kagen.sh

#echo "#3 Running"
#source $cwd/timekernelization-kagen-n100k-500k-sparse.sh

echo "#4 Solvers"
source $cmd/solvers-realworld.sh

echo "Done!"