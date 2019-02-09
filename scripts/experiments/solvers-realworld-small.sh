#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    echo "FULL CONCURRENCY POSSIBLE: " $config_use_full_concurrency


  #  allowed_total_time_seconds=2000
    allowed_total_time_seconds=21600
   # 21600=6*60*60 = 6 hours

    allowed_total_time_seconds=300

    mkdir -p $experiment_outdir/solvers/real-world-small/
    $builddir/./$selected_build -action "kernelization" -iterations 1 -f /mnt/d/master-thesis/implementation/scripts/experiments/../../data/thesis-tests/imgseg-x100000/imgseg_100098.txt.out \
                   -total-allowed-solver-time $allowed_total_time_seconds \
                    -do-weighted-reduction \
                    -support-weighted-result \
                    -dont-unweighted-reduction \
                    -number-of-threads 1 \
                    -locsearch-iterations 100 \
                    -exact-early-stop \
                    -benchmark-output $experiment_outdir/solvers/real-world-small/out > $experiment_outdir/solvers/real-world-small/out-exe
                                       # -no-mqlib -no-localsolver -do-signed-reduction -live-maxcut-analysis -support-weighted-result \

    rm $builddir/out-tmp*
}

func_localize


#to test solvers: ./benchmark-debug -action "kernelization" -iterations 1 -sample-kagen 1 -benchmark-output ../data/output/experiments/solvers/real-world/out-test -total-allowed-solver-time 2