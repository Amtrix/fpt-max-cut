#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

declare -a arr=("128" "200" "256" "512" "1024")

rm ../data/graphs-biqmac/*

for i in "${arr[@]}"
do
    ./$selected_build -action "kernelization" -iterations 1 -sample-kagen 50 -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*4)) -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/graph-gen/n"$i"/out > ../data/output/experiments/kernelization/graph-gen/n"$i"/out-exe \
                  -output-graphs-dir ../data/output/experiments/kernelization/graph-gen/n"$i"/graphs/ &

    check_and_wait_if_threadpool_full

    ./$selected_build -action "kernelization" -iterations 1 -sample-kagen 50 -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*4)) -total-allowed-solver-time -1 -support-weighted-result \
                  -benchmark-output ../data/output/experiments/kernelization/graph-gen/n"$i"w/out > ../data/output/experiments/kernelization/graph-gen/n"$i"w/out-exe \
                  -output-graphs-dir ../data/output/experiments/kernelization/graph-gen/n"$i"w/graphs/ &

    check_and_wait_if_threadpool_full
done

./$selected_build -action "kernelization" -iterations 1 -disk-suite real-world-small -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/graph-gen/real-world-small/out > ../data/output/experiments/kernelization/graph-gen/real-world-small/out-exe \
                  -output-graphs-dir ../data/output/experiments/kernelization/graph-gen/real-world-small/graphs/ &

check_and_wait_if_threadpool_full

./$selected_build -action "kernelization" -iterations 1 -disk-suite real-world-small -total-allowed-solver-time -1 -support-weighted-result \
                -benchmark-output ../data/output/experiments/kernelization/graph-gen/real-world-smallw/out > ../data/output/experiments/kernelization/graph-gen/real-world-smallw/out-exe \
                -output-graphs-dir ../data/output/experiments/kernelization/graph-gen/real-world-smallw/graphs/ &

check_and_wait_if_threadpool_full

./$selected_build -action "kernelization" -iterations 1 -disk-suite biqmac-rudy -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/graph-gen/biqmac-rudy/out > ../data/output/experiments/kernelization/graph-gen/biqmac-rudy/out-exe \
                  -output-graphs-dir ../data/output/experiments/kernelization/graph-gen/biqmac-rudy/graphs/ &

check_and_wait_if_threadpool_full

./$selected_build -action "kernelization" -iterations 1 -disk-suite biqmac-rudy -total-allowed-solver-time -1 -support-weighted-result \
                -benchmark-output ../data/output/experiments/kernelization/graph-gen/biqmac-rudyw/out > ../data/output/experiments/kernelization/graph-gen/biqmac-rudyw/out-exe \
                -output-graphs-dir ../data/output/experiments/kernelization/graph-gen/biqmac-rudyw/graphs/ &

wait_and_reset_threadpool