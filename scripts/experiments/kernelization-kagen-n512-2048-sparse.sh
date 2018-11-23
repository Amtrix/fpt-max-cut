#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

declare -a arr=("512" "2048")

for i in "${arr[@]}"
do
    ./$selected_build -action "kernelization" -iterations $num_iterations -sample-kagen $kagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*5)) -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/n"$i"/out > ../data/output/experiments/kernelization/n"$i"/out-exe &

    check_and_wait_if_threadpool_full

    ./$selected_build -action "kernelization" -iterations $num_iterations -sample-kagen $kagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*5)) -total-allowed-solver-time -1 -support-weighted-result \
                  -benchmark-output ../data/output/experiments/kernelization/n"$i"w/out > ../data/output/experiments/kernelization/n"$i"w/out-exe &

    check_and_wait_if_threadpool_full
done

./$selected_build -action "kernelization" -iterations 1 -sample-kagen 300 -num-nodes 2048 -num-edges-lo 0 -num-edges-hi $((i*10)) -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/n2048e/out > ../data/output/experiments/kernelization/n2048e/out-exe &

check_and_wait_if_threadpool_full

./$selected_build -action "kernelization" -iterations 1 -sample-kagen 300 -num-nodes 2048 -num-edges-lo 0 -num-edges-hi $((i*10)) -total-allowed-solver-time -1 -support-weighted-result \
                -benchmark-output ../data/output/experiments/kernelization/n2048ew/out > ../data/output/experiments/kernelization/n2048ew/out-exe &


wait_and_reset_threadpool