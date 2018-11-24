#!/bin/bash

rm plots/*.pdf

declare -a arr=("512" "512w" "2048" "2048w" "8000" "8000w")
declare -a arrl=("0.1" "0.2" "0.3" "0.4" "0.5")

for i in "${arr[@]}"
do
    Rscript plotting.r --file n$i/out --out plots/plot-n$i.pdf

    for j in "${arrl[@]}"
    do
        noperiodj="${j//./}"
        Rscript plotting.r --file n$i/out --out plots/plot-n$i-$noperiodj.pdf --loess $j
        Rscript plotting.r --file n$i/out --out plots/plot-n$i-$noperiodj-nop.pdf --loess $j --nopoints
    done
done