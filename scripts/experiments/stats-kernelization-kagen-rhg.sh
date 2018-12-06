#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
    
    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    declare -a arr=("512" "2048")
    declare -a arrrhg=("2.1" "3.1" "4.1" "5.1" "6.1" "7.1" "8.1")

    for i in "${arr[@]}"
    do
        for j in "${arrrhg[@]}"
        do
            $builddir/./$selected_build -action "kernelization" -iterations $knum_iterations -sample-kagen $kkagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*8)) -rhgfix $j -total-allowed-solver-time -1 \
                        -benchmark-output $experiment_outdir/kernelization/rhgfix/n"$i"_"$j"out > $experiment_outdir/kernelization/rhgfix/n"$i"_"$j"out-exe &

            check_and_wait_if_threadpool_full

            $builddir/./$selected_build -action "kernelization" -iterations $knum_iterations -sample-kagen $kkagen_instances -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*8)) -rhgfix $j -total-allowed-solver-time -1 -support-weighted-result \
                        -benchmark-output $experiment_outdir/kernelization/rhgfix/n"$i"_"$j"outw > $experiment_outdir/kernelization/rhgfix/n"$i"_"$j"outw-exe &
            
            check_and_wait_if_threadpool_full
        done
    done

    wait_and_reset_threadpool
}

func_localize