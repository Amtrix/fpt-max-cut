#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi


./$selected_build -action "kernelization" -iterations $num_iterations -disk-suite biqmac-rudy -total-allowed-solver-time -1 -support-weighted-result \
                  -benchmark-output ../data/output/experiments/kernelization/biqmac/out > ../data/output/experiments/kernelization/biqmac/out-exe

wait_and_reset_threadpool