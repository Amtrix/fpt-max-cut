#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    local num_iterations=1

    $builddir/./$selected_build -action "kernelization" -iterations $num_iterations -disk-suite biqmac-rudy -total-allowed-solver-time -1 -support-weighted-result \
                    -do-signed-reduction \
                    -do-weighted-reduction \
                    -force-weighted-result \
                    -benchmark-output $experiment_outdir/kernelization/biqmac/out > $experiment_outdir/kernelization/biqmac/out-exe

    wait_and_reset_threadpool
}

func_localize