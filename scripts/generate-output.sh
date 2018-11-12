#!/bin/bash
cd "${0%/*}"
cd ../build

bin_exe=benchmark

make $bin_exe

declare -a arr=("1" "5" "20" "50" "200" "500")

for i in "${arr[@]}"
do
    ./$bin_exe -action "kernelization" -sample-kagen $i -benchmark-output ../data/output/kernelization/out$i > ../data/output/kernelization/out$i-exe -num-edges-lo 4000 -num-edges-hi 40000 -num-nodes 8000 -locsearch-iterations 20 -mqlib-iterations 5 -locsolver-iterations 5 -total-allowed-time 0 
    cd ../data/output/kernelization
    cd ../data/output/kernelization
    Rscript plotting-compare.r --file out$i --out plot-cmp-out$i.pdf
    Rscript plotting-immense.r --file out$i --out plot-out$i.pdf
    cd ../../../build
done