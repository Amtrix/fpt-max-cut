#!/bin/bash

#set -o xtrace
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
        echo "Handle n: $n, nc: $nc"
        ./find-kernelization-general  -n $n -nc $nc -kernelization-efficiency -remove-iso \
                -output-path ../data/output/experiments/kernelization/small/iso-rem/aggregate > ../data/output/experiments/kernelization/small/iso-rem/out-exe_n"$n"_c"$nc".txt && echo "Done iso-rem" &
        
        ./find-kernelization-general  -n $n -nc $nc -remove-iso \
                -output-path ../data/output/experiments/kernelization/small/iso-rem-no-kernelization/aggregate > ../data/output/experiments/kernelization/small/iso-rem-no-kernelization/out-exe_n"$n"_c"$nc".txt && echo "Done iso-rem-no-kernelization" &
        if [ $n -lt 7 ] ; then
                dorough=""
        else
                dorough="-dorough"
        fi

        ./find-kernelization-general  -n $n -nc $nc -kernelization-efficiency $dorough \
                -output-path ../data/output/experiments/kernelization/small/iso-kept/aggregate > ../data/output/experiments/kernelization/small/iso-kept/out-exe_n"$n"_c"$nc".txt && echo "Done iso-kept" &

        ./find-kernelization-general  -n $n -nc $nc $dorough \
                -output-path ../data/output/experiments/kernelization/small/iso-kept-no-kernelization/aggregate > ../data/output/experiments/kernelization/small/iso-kept-no-kernelization/out-exe_n"$n"_c"$nc".txt && echo "Done iso-kept-no-kernelization" &

        echo "... wait"
        wait
    done
done