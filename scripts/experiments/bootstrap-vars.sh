#!/bin/bash

func_localize() {
    set -o xtrace
    local cwdboot="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
    builddir=$cwdboot/../../build

    # CONFIG FOR TEST CASES IS HERE
    experiment_outdir=$cwdboot/../../output/experiments
    num_iterations=5
    knum_iterations=10
    kkagen_instances=150 # kernelization stats (stats-kernelization-kagen)
    ################################
}

func_localize