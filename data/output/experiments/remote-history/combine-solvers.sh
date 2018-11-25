#/bin/bash

declare -a arr=("out-maxcut_live-localsolver" "out-maxcut_live-localsolver-kernelized")

for i in "${arr[@]}"
do

    cat ./1/experiments/solvers/real-world/$i > ../solvers/real-world/$i
    tail -n +2 ./2/experiments/solvers/real-world/$i >> ../solvers/real-world/$i
    tail -n +2 ./3/experiments/solvers/real-world/$i >> ../solvers/real-world/$i
done