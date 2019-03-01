#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    echo "FULL CONCURRENCY POSSIBLE: " $config_use_full_concurrency


    #10 hours:
    allowed_total_time_seconds=1800
    allowed_total_time_seconds=300

    mkdir -p $experiment_outdir/solvers/real-world-live/
    $builddir/./$selected_build -action "kernelization" -iterations 100 -f $thesis_tests/real-world-live/ca-coauthors-dblp.mtx  \
                    -total-allowed-solver-time-range $allowed_total_time_seconds \
                    -number-of-threads 1 \
                    -number-of-iter-threads 5 \
                    -locsearch-iterations 0 \
                    -no-localsolver \
                    -no-biqmac \
                    -force-weighted-result \
                    -benchmark-output $experiment_outdir/solvers/real-world-live/out > $experiment_outdir/solvers/real-world-live/out-exe
                                       # -no-mqlib -no-localsolver -do-signed-reduction -live-maxcut-analysis -force-weighted-result \
                                       # number-of-threads 8 for KIT pc

    rm $builddir/out-tmp*
}

func_localize


#to test solvers: ./benchmark-debug -action "kernelization" -iterations 1 -sample-kagen 1 -benchmark-output ../data/output/experiments/solvers/real-world/out-test -total-allowed-solver-time 2