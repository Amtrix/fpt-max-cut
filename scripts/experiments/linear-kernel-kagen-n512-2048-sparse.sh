#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

declare -a arr=("512" "2048")

for i in "${arr[@]}"
do
    ./$selected_build -action "linear-kernel" -iterations $num_iterations -sample-kagen $kagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*8)) \
                  -benchmark-output ../data/output/experiments/linear-kernel/n"$i"/out > ../data/output/experiments/linear-kernel/n"$i"/out-exe &

    check_and_wait_if_threadpool_full
done

#times of no concern here, use 1 iteration
./$selected_build -action "linear-kernel" -iterations 1 -sample-kagen 300 -num-nodes 2048 -num-edges-lo 0 -num-edges-hi $((i*8)) \
                  -benchmark-output ../data/output/experiments/linear-kernel/n2048e/out > ../data/output/experiments/linear-kernel/n2048e/out-exe &

wait_and_reset_threadpool