#!/bin/bash

cwd=$(pwd)
source ./bootstrap.sh

echo "#0 Running"
source $cwd/kernelization-kagen-rhg-n512-2048-sparse.sh

echo "#1 Running"
source $cwd/kernelization-kagen-n512-2048-sparse.sh

echo "#2 Running"
source $cwd/kernelization-kagen-n100k-500k-sparse.sh

echo "#3 Running"
source $cwd/linear-kernel-kagen-n512-2048-sparse.sh

echo "Done!"