#!/bin/bash

rm plots/*.pdf

declare -a arr=("512" "2048")

for i in "${arr[@]}"
do

    Rscript plotting.r --file n$i/out --out plots/plot-n$i.pdf
    Rscript plotting.r --file n$i/out --out plots/plot-n$i-01.pdf --loess 0.1
    Rscript plotting.r --file n$i/out --out plots/plot-n$i-02.pdf --loess 0.2
    Rscript plotting.r --file n$i/out --out plots/plot-n$i-05.pdf --loess 0.5
    Rscript plotting.r --file n$i/out --out plots/plot-n$i-07.pdf --loess 0.7

    Rscript plotting.r --file n$i/out --out plots/plot-n$i-01-nop.pdf --loess 0.1 --nopoints
    Rscript plotting.r --file n$i/out --out plots/plot-n$i-02-nop.pdf --loess 0.2 --nopoints
    Rscript plotting.r --file n$i/out --out plots/plot-n$i-05-nop.pdf --loess 0.5 --nopoints
    Rscript plotting.r --file n$i/out --out plots/plot-n$i-07-nop.pdf --loess 0.7 --nopoints
done