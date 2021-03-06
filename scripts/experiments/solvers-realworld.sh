#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi


  #  allowed_total_time_seconds=2000
    allowed_total_time_seconds=300

    mkdir -p $experiment_outdir/solvers/real-world/
    $builddir/./$selected_build -action "kernelization" -iterations $num_iterations -disk-suite realworld -live-maxcut-analysis -total-allowed-solver-time $allowed_total_time_seconds -no-biqmac \
                    -benchmark-output $experiment_outdir/solvers/real-world/out > $experiment_outdir/solvers/real-world/out-exe
}

func_localize


#to test solvers: ./benchmark-debug -action "kernelization" -iterations 1 -sample-kagen 1 -benchmark-output ../data/output/experiments/solvers/real-world/out-test -total-allowed-solver-time 2


#biqmac no brainer for cases of this size!