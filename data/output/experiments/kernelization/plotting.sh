#!/bin/bash

declare -a arr=("512" "512w" "2048" "2048w" "2048e" "2048ew")

for i in "${arr[@]}"
do

    Rscript plotting.r --file n$i/out --out plot-n$i.pdf
    Rscript plotting.r --file n$i/out --out plot-n$i-01.pdf --loess 0.1
    Rscript plotting.r --file n$i/out --out plot-n$i-03.pdf --loess 0.3
    Rscript plotting.r --file n$i/out --out plot-n$i-05.pdf --loess 0.5
    Rscript plotting.r --file n$i/out --out plot-n$i-07.pdf --loess 0.7

    Rscript plotting.r --file n$i/out --out plot-n$i-01-nop.pdf --loess 0.1 --nopoints
    Rscript plotting.r --file n$i/out --out plot-n$i-03-nop.pdf --loess 0.3 --nopoints
    Rscript plotting.r --file n$i/out --out plot-n$i-05-nop.pdf --loess 0.5 --nopoints
    Rscript plotting.r --file n$i/out --out plot-n$i-07-nop.pdf --loess 0.7 --nopoints
done