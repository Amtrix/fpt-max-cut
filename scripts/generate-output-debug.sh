#!/bin/bash
cd "${0%/*}"
cd ../build

make benchmark-debug

declare -a arr=("1" "5")

for i in "${arr[@]}"
do
    ./benchmark-debug -action "kernelization" -sample-kagen $i -benchmark-output ../data/output/kernelization/out$i > ../data/output/kernelization/out$i-exe -num-edges-lo 10000 -num-edges-hi 24000 -num-nodes 8000 -locsearch-iterations 200
    cd ../data/output/kernelization
    Rscript plotting-compare.r --file out$i --out plot-cmp-out$i.pdf
    cd ../../../build
done