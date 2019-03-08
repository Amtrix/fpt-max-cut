#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    if [ ! "$bootstrap_done" = true ] ; then
        source $cwd/bootstrap.sh
    fi

    echo "FULL CONCURRENCY POSSIBLE: " $config_use_full_concurrency


    #3 hours:
    local allowed_total_time_seconds=10800

        
    mkdir -p $experiment_outdir/solvers/real-world/
    $builddir/./$selected_build -action "kernelization" -fdir $thesis_tests/real-world  \
                    -total-allowed-solver-time $allowed_total_time_seconds \
                    -number-of-threads 10 \
                    -locsearch-iterations 0 -no-biqmac -no-localsolver \
                    -use-fast-kernelization \
                    -benchmark-output $experiment_outdir/solvers/real-world/out > $experiment_outdir/solvers/real-world/out-exe
                                    # -no-mqlib -no-localsolver -do-signed-reduction -live-maxcut-analysis -force-weighted-result \
                                    # number-of-threads 8 for KIT pc

    

    rm $builddir/out-tmp*
}

func_localize


#to test solvers: ./benchmark-debug -action "kernelization" -iterations 1 -sample-kagen 1 -benchmark-output ../data/output/experiments/solvers/real-world/out-test -total-allowed-solver-time 2
