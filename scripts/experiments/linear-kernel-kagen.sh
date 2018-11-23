#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

declare -a arr=("512" "2048")

for i in "${arr[@]}"
do
    ./$selected_build -action "linear-kernel" -iterations $knum_iterations -sample-kagen 300 -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*8)) \
                  -benchmark-output ../data/output/experiments/linear-kernel/n"$i"/out > ../data/output/experiments/linear-kernel/n"$i"/out-exe &

    check_and_wait_if_threadpool_full
done

wait_and_reset_threadpool