#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    declare -a arr=("128" "200" "256" "512" "1024")

    rm -r $experiment_outdir/kernelization/graph-gen

    for i in "${arr[@]}"
    do
        mkdir -p $experiment_outdir/kernelization/graph-gen/n"$i"/graphs
        $builddir/./$selected_build -action "kernelization" -iterations $num_iterations -sample-kagen 50 -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*4)) -total-allowed-solver-time -1 \
                    -benchmark-output $experiment_outdir/kernelization/graph-gen/n"$i"/out > $experiment_outdir/kernelization/graph-gen/n"$i"/out-exe \
                    -output-graphs-dir $experiment_outdir/kernelization/graph-gen/n"$i"/graphs/ &

        check_and_wait_if_threadpool_full

        mkdir -p $experiment_outdir/kernelization/graph-gen/n"$i"w/graphs
        $builddir/./$selected_build -action "kernelization" -iterations $num_iterations -sample-kagen 50 -num-nodes $i -num-edges-lo 0 -num-edges-hi $((i*4)) -total-allowed-solver-time -1 -support-weighted-result \
                    -benchmark-output $experiment_outdir/kernelization/graph-gen/n"$i"w/out > $experiment_outdir/kernelization/graph-gen/n"$i"w/out-exe \
                    -output-graphs-dir $experiment_outdir/kernelization/graph-gen/n"$i"w/graphs/ &

        check_and_wait_if_threadpool_full
    done

    mkdir -p $experiment_outdir/kernelization/graph-gen/real-world-small/graphs
    $builddir/./$selected_build -action "kernelization" -iterations $num_iterations -disk-suite real-world-small -total-allowed-solver-time -1 \
                    -benchmark-output $experiment_outdir/kernelization/graph-gen/real-world-small/out > $experiment_outdir/kernelization/graph-gen/real-world-small/out-exe \
                    -output-graphs-dir $experiment_outdir/kernelization/graph-gen/real-world-small/graphs/ &

    check_and_wait_if_threadpool_full

    mkdir -p $experiment_outdir/kernelization/graph-gen/real-world-smallw/graphs
    $builddir/./$selected_build -action "kernelization" -iterations $num_iterations -disk-suite real-world-small -total-allowed-solver-time -1 -support-weighted-result \
                    -benchmark-output $experiment_outdir/kernelization/graph-gen/real-world-smallw/out > $experiment_outdir/kernelization/graph-gen/real-world-smallw/out-exe \
                    -output-graphs-dir $experiment_outdir/kernelization/graph-gen/real-world-smallw/graphs/ &

    check_and_wait_if_threadpool_full

    mkdir -p $experiment_outdir/kernelization/graph-gen/biqmac-rudy/graphs
    $builddir/./$selected_build -action "kernelization" -iterations $num_iterations -disk-suite biqmac-rudy -total-allowed-solver-time -1 \
                    -benchmark-output $experiment_outdir/kernelization/graph-gen/biqmac-rudy/out > $experiment_outdir/kernelization/graph-gen/biqmac-rudy/out-exe \
                    -output-graphs-dir $experiment_outdir/kernelization/graph-gen/biqmac-rudy/graphs/ &

    check_and_wait_if_threadpool_full

    mkdir -p $experiment_outdir/kernelization/graph-gen/biqmac-rudyw/graphs
    $builddir/./$selected_build -action "kernelization" -iterations $num_iterations -disk-suite biqmac-rudy -total-allowed-solver-time -1 -support-weighted-result \
                    -benchmark-output $experiment_outdir/kernelization/graph-gen/biqmac-rudyw/out > $experiment_outdir/kernelization/graph-gen/biqmac-rudyw/out-exe \
                    -output-graphs-dir $experiment_outdir/kernelization/graph-gen/biqmac-rudyw/graphs/ &

    wait_and_reset_threadpool
}

func_localize