#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    mkdir -p $experiment_outdir/linear-kernel/biqmac
    $builddir/./$selected_build -action "linear-kernel" -iterations $num_iterations -disk-suite biqmac-rudy -do-reduce "yes" \
                -benchmark-output $experiment_outdir/linear-kernel/biqmac/out > $experiment_outdir/linear-kernel/biqmac/out-exe &
    check_and_wait_if_threadpool_full

    mkdir -p $experiment_outdir/linear-kernel/real-world-small
    $builddir/./$selected_build -action "linear-kernel" -iterations $num_iterations -disk-suite real-world-small -do-reduce "yes" \
                -benchmark-output $experiment_outdir/linear-kernel/real-world-small/out > $experiment_outdir/linear-kernel/real-world-small/out-exe &
    check_and_wait_if_threadpool_full

    wait_and_reset_threadpool
}

func_localize