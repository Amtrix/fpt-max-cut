#!/bin/bash
cd "${0%/*}"
cd ../build

declare -a arr=("1" "5" "20" "50" "200" "500")

for i in "${arr[@]}"
do
    #./$bin_exe -action "kernelization" -sample-kagen $i -benchmark-output ../data/output/kernelization/out$i > ../data/output/kernelization/out$i-exe -num-edges-lo 4000 -num-edges-hi 40000 -num-nodes 8000 -locsearch-iterations 200 -mqlib-iterations 5 -locsolver-iterations 5
    cd ../data/output/kernelization
    Rscript plotting-compare.r --file out$i --out plot-cmp-out$i.pdf
    Rscript plotting-immense.r --file out$i --out plot-out$i.pdf
    cd ../../../build
done