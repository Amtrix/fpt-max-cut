#!/bin/bash

set -o xtrace

cd "${0%/*}"
cd ../../build

if [ ! "$bootstrap_done" = true ] ; then
    cmake ../

    # CONFIG FOR TEST CASES IS HERE
    bootstrap_done=true
    num_iterations=10
    kagen_instances=50
    ################################

    while [[ $selected_build = "" ]]; do
        read -p "Select the build (benchmark or benchmark-debug): " selected_build
    done

    make $selected_build
else
    echo "Bootstrap called twice!"
fi