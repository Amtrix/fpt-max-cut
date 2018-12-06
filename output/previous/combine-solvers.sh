#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
    local historydir=$cwd/remote-history
    local combinedir=$cwd/remote-combined

    declare -a arr=("out-maxcut_live-localsolver" "out-maxcut_live-localsolver-kernelized" "out")

    mkdir -p $cwd/remote-combined
    rm $cwd/remote-combined/* -r

    mkdir -p $cwd/remote-combined/solvers/real-world

    for i in "${arr[@]}"
    do

        cat $historydir/3/experiments/solvers/real-world/$i > $combinedir/solvers/real-world/$i
        tail -n +2 $historydir/4/experiments/solvers/real-world/$i >> $combinedir/solvers/real-world/$i
        #tail -n +2 ./3/experiments/solvers/real-world/$i >> ../solvers/real-world/$i
    done
}

func_localize