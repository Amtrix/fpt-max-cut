#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    declare -a arr=("512" "2048")
    # "8000")

    declare subtype=""
    #declare subtype="/missing-r8-with-signed"

    local knum_iterations=1

    for i in "${arr[@]}"
    do
        experiment_outdir_fin=$experiment_outdir/kernelization/n"$i"$subtype
        mkdir -p $experiment_outdir/kernelization/n"$i"/$subtype
        $builddir/./$selected_build -action "kernelization" -iterations $knum_iterations -sample-kagen $kkagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*8)) -total-allowed-solver-time -1 \
                    -benchmark-output $experiment_outdir_fin/out > $experiment_outdir_fin/out-exe &

        check_and_wait_if_threadpool_full

        experiment_outdir_fin=$experiment_outdir/kernelization/n"$i"w$subtype
        mkdir -p $experiment_outdir_fin
        $builddir/./$selected_build -action "kernelization" -iterations $knum_iterations -sample-kagen $kkagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*8)) -total-allowed-solver-time -1 -support-weighted-result \
                    -benchmark-output $experiment_outdir_fin/out > $experiment_outdir_fin/out-exe &

        check_and_wait_if_threadpool_full
    done

    wait_and_reset_threadpool
}

func_localize