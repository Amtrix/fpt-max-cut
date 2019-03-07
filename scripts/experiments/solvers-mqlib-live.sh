#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    echo "FULL CONCURRENCY POSSIBLE: " $config_use_full_concurrency


    #10 hours:
    allowed_total_time_seconds=600

    echo "" > $experiment_outdir/solvers/real-world-live/out

    local iterations=600
    local idtime=20
    for i in $(seq 0 $iterations)
    do
        check_and_wait_if_threadpool_full
        idtime=$((idtime + 1))

        local optflags=""
        optflags="$optflags -no-output-init"
        
        mkdir -p $experiment_outdir/solvers/real-world-live/
        $builddir/./$selected_build -action "kernelization" -fdir $thesis_tests/real-world-live  \
                        -iteration-offset $i \
                        -total-allowed-solver-time $idtime \
                        -number-of-threads 1 \
                        -number-of-iter-threads 1 \
                        -locsearch-iterations 0 -no-biqmac -no-localsolver \
                        -use-fast-kernelization \
                        $optflags \
                        -benchmark-output $experiment_outdir/solvers/real-world-live/out-$threadcnt4946 > $experiment_outdir/solvers/real-world-live/out-$threadcnt4946-exe &
                                        # -no-mqlib -no-localsolver -do-signed-reduction -live-maxcut-analysis -force-weighted-result \
                                        # number-of-threads 8 for KIT pc
    done

    for i in $(seq 0 $threads)
    do
        cat $experiment_outdir/solvers/real-world-live/out-$i >> $experiment_outdir/solvers/real-world-live/out
        cat $experiment_outdir/solvers/real-world-live/out-$i-exe >> $experiment_outdir/solvers/real-world-live/out-exe
        wait
        rm $experiment_outdir/solvers/real-world-live/out-$i
        rm $experiment_outdir/solvers/real-world-live/out-$i-exe
        rm $experiment_outdir/solvers/real-world-live/out-$i-avg
    done

    

    rm $builddir/out-tmp*
}

func_localize


#to test solvers: ./benchmark-debug -action "kernelization" -iterations 1 -sample-kagen 1 -benchmark-output ../data/output/experiments/solvers/real-world/out-test -total-allowed-solver-time 2
