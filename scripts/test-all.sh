#!/bin/bash

summary=""

call_test() {
    $builddir/./$1

    retVal=$?
    if [ $retVal -ne 0 ]; then
        #exit 1
        summary=$(echo "$summary"$1" = FAIL\n")
       # summary="$summary"$1" = FAIL\n"
    else
        summary=$(echo "$summary"$1" = OK\n")
    fi
}

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
    builddir=$cwd/../build

    cd $builddir

    make test-*

    set -o xtrace

    call_test test-articulation-and-biconnected
    call_test test-graph-functionality
    call_test test-kernelization-all
    call_test test-kernelization-auto
}

func_localize

echo -e "Summary:\n$summary"