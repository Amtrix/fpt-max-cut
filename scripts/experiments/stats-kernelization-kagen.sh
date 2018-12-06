#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    declare -a arr=("512" "2048" "8000")

    for i in "${arr[@]}"
    do
        $builddir/./$selected_build -action "kernelization" -iterations $knum_iterations -sample-kagen $kkagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*8)) -total-allowed-solver-time -1 \
                    -benchmark-output $experiment_outdir/kernelization/n"$i"/out > $experiment_outdir/kernelization/n"$i"/out-exe &

        check_and_wait_if_threadpool_full

        $builddir/./$selected_build -action "kernelization" -iterations $knum_iterations -sample-kagen $kkagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*8)) -total-allowed-solver-time -1 -support-weighted-result \
                    -benchmark-output $experiment_outdir/kernelization/n"$i"w/out > $experiment_outdir/kernelization/n"$i"w/out-exe &

        check_and_wait_if_threadpool_full
    done

    wait_and_reset_threadpool
}

func_localize