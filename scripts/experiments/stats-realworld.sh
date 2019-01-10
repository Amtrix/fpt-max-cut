#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi


    mkdir -p $experiment_outdir/solvers/real-world/
    $builddir/./$selected_build -action "kernelization" -iterations 1 -disk-suite realworld -live-maxcut-analysis -total-allowed-solver-time -1 \
                    -benchmark-output $experiment_outdir/solvers/real-world/out > $experiment_outdir/solvers/real-world/out-exe &
    
    check_and_wait_if_threadpool_full

    mkdir -p $experiment_outdir/solvers/real-world-small/
    $builddir/./$selected_build -action "kernelization" -iterations 1 -disk-suite real-world-small -live-maxcut-analysis -total-allowed-solver-time -1 \
                    -benchmark-output $experiment_outdir/solvers/real-world-small/out > $experiment_outdir/solvers/real-world-small/out-exe &
    
    check_and_wait_if_threadpool_full
    
    mkdir -p $experiment_outdir/solvers/real-world-special/
    $builddir/./$selected_build -action "kernelization" -iterations 1 -disk-suite real-world-special -live-maxcut-analysis -total-allowed-solver-time -1 -support-weighted-result \
                    -benchmark-output $experiment_outdir/solvers/real-world-special/out > $experiment_outdir/solvers/real-world-special/out-exe &
    
    check_and_wait_if_threadpool_full
}

func_localize


#to test solvers: ./benchmark-debug -action "kernelization" -iterations 1 -sample-kagen 1 -benchmark-output $experiment_outdir/solvers/real-world/out-test -total-allowed-solver-time 2