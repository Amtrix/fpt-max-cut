#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

declare -a arr=("128" "256" "512" "1024")

rm ../data/graphs-biqmac/*

for i in "${arr[@]}"
do
    ./$selected_build -action "kernelization" -iterations 1 -sample-kagen 30 -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*3)) -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/graph-gen/n"$i"/out > ../data/output/experiments/kernelization/graph-gen/n"$i"/out-exe \
                  -output-graphs-dir ../data/output/experiments/kernelization/graph-gen/n"$i"/graphs/ &

    check_and_wait_if_threadpool_full

    ./$selected_build -action "kernelization" -iterations 1 -sample-kagen 30 -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*3)) -total-allowed-solver-time -1 -support-weighted-result \
                  -benchmark-output ../data/output/experiments/kernelization/graph-gen/n"$i"w/out > ../data/output/experiments/kernelization/graph-gen/n"$i"w/out-exe \
                  -output-graphs-dir ../data/output/experiments/kernelization/graph-gen/n"$i"w/graphs/ &

    check_and_wait_if_threadpool_full
done

wait_and_reset_threadpool