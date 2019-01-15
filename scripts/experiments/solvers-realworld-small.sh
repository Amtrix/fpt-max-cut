#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    local iterations=5

  #  allowed_total_time_seconds=2000
    allowed_total_time_seconds=21600

    for ((n=1;n<=$iterations;n++))
    do
        mkdir -p $experiment_outdir/solvers/real-world-small/
        $builddir/./$selected_build -action "kernelization" -iterations 1 -disk-suite real-world-small -total-allowed-solver-time $allowed_total_time_seconds \
                        -do-signed-reduction \
                        -benchmark-output $experiment_outdir/solvers/real-world-small/out$i > $experiment_outdir/solvers/real-world-small/out-exe$i &
                                        # -no-mqlib -no-localsolver -do-signed-reduction -live-maxcut-analysis \
        
        check_and_wait_if_threadpool_full
    done

    wait_and_reset_threadpool
}

func_localize


#to test solvers: ./benchmark-debug -action "kernelization" -iterations 1 -sample-kagen 1 -benchmark-output ../data/output/experiments/solvers/real-world/out-test -total-allowed-solver-time 2