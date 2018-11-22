#!/bin/bash

set -o xtrace

cd "${0%/*}"
cd ../../build

cnt=0
check_and_wait_if_threadpool_full() {
    cnt=$((cnt+1))
    echo $cnt

    if [ $cnt -eq $threads ] ; then
            wait
            cnt=0
    fi
}

wait_and_reset_threadpool() {
    wait
    cnt=0
}

if [ ! "$bootstrap_done" = true ] ; then
    cmake ../

    # CONFIG FOR TEST CASES IS HERE
    bootstrap_done=true
    num_iterations=5
    kagen_instances=50
    ################################

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