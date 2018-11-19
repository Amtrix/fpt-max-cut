#!/bin/bash

source ./bootstrap.sh

./$selected_build -action "linear-kernel" -iterations 10 -sample-kagen 20 -num-nodes 512 -num-edges-lo 0 -num-edges-hi 2500 -benchmark-output ../data/output/experiments/linear-kernel/n512/out > ../data/output/experiments/linear-kernel/n512/out-exe
./$selected_build -action "linear-kernel" -iterations 10 -sample-kagen 20 -num-nodes 2048 -num-edges-lo 0 -num-edges-hi 10000 -benchmark-output ../data/output/experiments/linear-kernel/n2048/out > ../data/output/experiments/linear-kernel/n2048/out-exe