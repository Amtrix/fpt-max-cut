#!/bin/bash
ls
cd "${0%/*}"
cd ../build

make test-*

set -o xtrace
./test-articulation-and-biconnected
./test-graph-functionality
./test-kernelization-all
./test-kernelization-auto