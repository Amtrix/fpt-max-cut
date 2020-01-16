#/bin/bash

func_localize() {
    #local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    #sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    #sudo apt-get -qq update
    #sudo apt-get install gcc-7 g++-7 libopenmpi-dev libcgal-dev libcgal-qt5-dev libsparsehash-dev 

    if [[ ! -d "build" ]]
    then
            ./build.sh
    fi
    #git submodule update --init --recursive
    #cd $cwd/solvers/MQLIb
    #make

    #mkdir $cwd/build
    #cd $cwd/build
    #cmake ../
    #make benchmark

    rm -rf examples_output
    mkdir examples_output
    experiment_outdir=examples_output/n1024

    mkdir -p $experiment_outdir
    ./build/./benchmark -action "kernelization" -iterations 1 -sample-kagen 16 -num-nodes 64 -num-edges-lo 0 -num-edges-hi $((64*4)) -total-allowed-solver-time 1 \
                -number-of-threads 4  -benchmark-output $experiment_outdir/out > $experiment_outdir/out-exe

    experiment_outdir=examples_output/example
    mkdir -p $experiment_outdir
    ./build/./benchmark -action "kernelization" -iterations 1 -f $cwd/data/example-graphs/example -total-allowed-solver-time 1 \
                -number-of-threads 4  -benchmark-output $experiment_outdir/out > $experiment_outdir/out-exe

    experiment_outdir=examples_output/example.graph
    mkdir -p $experiment_outdir
    ./build/./benchmark -action "kernelization" -iterations 1 -f $cwd/data/example-graphs/example.graph -total-allowed-solver-time 1 \
                -number-of-threads 4  -benchmark-output $experiment_outdir/out > $experiment_outdir/out-exe
    
    experiment_outdir=examples_output/example.edges
    mkdir -p $experiment_outdir
    ./build/./benchmark -action "kernelization" -iterations 1 -f $cwd/data/example-graphs/example.edges -total-allowed-solver-time 1 \
                -number-of-threads 4  -benchmark-output $experiment_outdir/out > $experiment_outdir/out-exe


    cd $cwd
}

func_localize
