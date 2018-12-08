#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
    builddir=$cwd/../build

    cd $builddir

    make test-*

    set -o xtrace
    $builddir/./test-articulation-and-biconnected
    $builddir/./test-graph-functionality
    $builddir/./test-kernelization-all
    $builddir/./test-kernelization-auto
}

func_localize