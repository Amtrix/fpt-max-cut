#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

declare -a arr=("512" "2048")
declare -a arrrhg=("2.1" "3.1" "4.1" "5.1" "6.1" "7.1" "8.1")

for i in "${arr[@]}"
do
    for j in "${arrrhg[@]}"
    do
        ./$selected_build -action "kernelization" -iterations $num_iterations -sample-kagen $kagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*5)) -rhgfix $j -total-allowed-solver-time -1 \
                    -benchmark-output ../data/output/experiments/kernelization/rhgfix/n"$i"_"$j"out > ../data/output/experiments/kernelization/rhgfix/n"$i"_"$j"out-exe

        ./$selected_build -action "kernelization" -iterations $num_iterations -sample-kagen $kagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*5)) -rhgfix $j -total-allowed-solver-time -1 -support-weighted-result \
                    -benchmark-output ../data/output/experiments/kernelization/rhgfix/n"$i"_"$j"outw > ../data/output/experiments/kernelization/rhgfix/n"$i"_"$j"outw-exe
    done
done