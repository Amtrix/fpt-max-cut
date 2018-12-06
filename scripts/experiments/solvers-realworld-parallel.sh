#!/bin/bash

search_dir=$(pwd)

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

for entry in "$search_dir"/../../data/thesis-tests/real-world/*
do
    ./$selected_build -action "kernelization" -iterations $num_iterations -f $entry -live-maxcut-analysis \
                  -benchmark-output ../data/output/experiments/solvers/real-world/out-$(basename $entry) > ../data/output/experiments/solvers/real-world/out-$(basename $entry)-exe &
    
    check_and_wait_if_threadpool_full
done

wait_and_reset_threadpool


#to test solvers: ./benchmark-debug -action "kernelization" -iterations 1 -sample-kagen 1 -benchmark-output ../data/output/experiments/solvers/real-world/out-test -total-allowed-solver-time 2

# THIS DOES NOT WORK WITH LOCALSOLVER AS THEY ONLY PROVIDE ONE TOKEN PER LICENSE!