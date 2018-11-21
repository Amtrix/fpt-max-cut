#!/bin/bash

if [ ! "$bootstrap_done" = true ] ; then
    source ./bootstrap.sh
fi

./$selected_build -action "kernelization" -iterations $num_iterations -disk-suite realworld \
                  -benchmark-output ../data/output/experiments/solvers/real-world/out > ../data/output/experiments/solvers/real-world/out-exe