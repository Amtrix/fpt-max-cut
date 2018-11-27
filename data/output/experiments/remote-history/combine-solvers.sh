#/bin/bash

declare -a arr=("out-maxcut_live-localsolver" "out-maxcut_live-localsolver-kernelized" "out")

for i in "${arr[@]}"
do

    cat ./3/experiments/solvers/real-world/$i > ../solvers/real-world/$i
    tail -n +2 ./4/experiments/solvers/real-world/$i >> ../solvers/real-world/$i
    #tail -n +2 ./3/experiments/solvers/real-world/$i >> ../solvers/real-world/$i
done