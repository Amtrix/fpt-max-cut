#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

declare -a arr=("100000" "500000")

for i in "${arr[@]}"
do
    ./$selected_build -action "kernelization" -iterations 2 -sample-kagen 30 -num-nodes $i -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/n"$i"/out > ../data/output/experiments/kernelization/n"$i"/out-exe

    ./$selected_build -action "kernelization" -iterations 2 -sample-kagen 30 -num-nodes $i  -total-allowed-solver-time -1 -support-weighted-result \
                  -benchmark-output ../data/output/experiments/kernelization/n"$i"w/out > ../data/output/experiments/kernelization/n"$i"w/out-exe
done