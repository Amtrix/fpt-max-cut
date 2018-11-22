#!/bin/bash
cd "${0%/*}"
cd ../../build
make find-kernelization-general

declare -a arr=("iso-kept/aggregate" "iso-rem/aggregate" "iso-rem-no-kernelization/aggregate" "iso-kept-no-kernelization/aggregate")

for i in "${arr[@]}"
do
    printf "%20s%20s%20s%20s%20s\n" "#n" "#nc" "#numgraphs" "#numclasses" "#coverage" > ../data/output/experiments/kernelization/small/$i
done

for ((n=2;n<=7;n++))
do
    for ((nc=0;nc<=n;nc++))
    do
        ./find-kernelization-general  -n $n -nc $nc -kernelization-efficiency -remove-iso \
                -output-path ../data/output/experiments/kernelization/small/iso-rem/aggregate > ../data/output/experiments/kernelization/small/iso-rem/out-exe_n"$n"_c"$nc".txt
        ./find-kernelization-general  -n $n -nc $nc -kernelization-efficiency \
                -output-path ../data/output/experiments/kernelization/small/iso-kept/aggregate > ../data/output/experiments/kernelization/small/iso-kept/out-exe_n"$n"_c"$nc".txt
        ./find-kernelization-general  -n $n -nc $nc -remove-iso \
                -output-path ../data/output/experiments/kernelization/small/iso-rem-no-kernelization/aggregate > ../data/output/experiments/kernelization/small/iso-rem-no-kernelization/out-exe_n"$n"_c"$nc".txt
        ./find-kernelization-general  -n $n -nc $nc \
                -output-path ../data/output/experiments/kernelization/small/iso-kept-no-kernelization/aggregate > ../data/output/experiments/kernelization/small/iso-kept-no-kernelization/out-exe_n"$n"_c"$nc".txt
    done
done