#!/bin/bash

func_localize() {
    set -o xtrace
    local cwdboot="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
    source $cwdboot/bootstrap-vars.sh

    threadcnt4946=0
    check_and_wait_if_threadpool_full() {
        threadcnt4946=$((threadcnt4946+1))

        if [ $threadcnt4946 -eq $threads ] ; then
            wait
            threadcnt4946=0
        fi
    }

    wait_and_reset_threadpool() {
        wait
        threadcnt4946=0
    }

    if [ ! "$bootstrap_done" = true ] ; then
        cd $builddir
        cmake ../

        source $cwdboot/cmake.config.generated.sh

        while [[ $selected_build != "benchmark" && $selected_build != "benchmark-debug" ]]; do
            read -p "Select the build (benchmark or benchmark-debug): " selected_build
        done

        while ! [[ "$threads" =~ ^[0-9]+$ ]]; do
            read -p "Number of threads to handle for script handling " threads
        done

        make $selected_build
    else
        echo "Bootstrap called twice!"
    fi
}

func_localize