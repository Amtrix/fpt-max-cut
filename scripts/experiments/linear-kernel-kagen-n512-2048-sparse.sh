#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

declare -a arr=("512" "2048")

for i in "${arr[@]}"
do
    ./$selected_build -action "linear-kernel" -iterations $num_iterations -sample-kagen $kagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*5)) \
                  -benchmark-output ../data/output/experiments/kernelization/n"$i"/out > ../data/output/experiments/kernelization/n"$i"/out-exe
done
