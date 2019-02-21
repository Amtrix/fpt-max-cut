#!/bin/bash

#
#cd "${0%/*}"


func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
    source $cwd/bootstrap-vars.sh

    bin=find-kernelization-general-debug

    cd $builddir
    make $bin
    cd $cwd

    declare -a arr=("iso-kept/aggregate" "iso-rem/aggregate" "iso-rem-no-kernelization/aggregate" "iso-kept-no-kernelization/aggregate")

    for i in "${arr[@]}"
    do
        printf "%20s%20s%20s%20s%20s\n" "#n" "#nc" "#numgraphs" "#numclasses" "#coverage" > $experiment_outdir/kernelization/small/$i
    done

    set +o xtrace

    for ((n=2;n<=7;n++))
    do
        for ((nc=0;nc<=n;nc++))
        do
            mkdir -p $experiment_outdir/kernelization/small/iso-rem
            mkdir -p $experiment_outdir/kernelization/small/iso-rem-no-kernelization
            mkdir -p $experiment_outdir/kernelization/small/iso-kept
            mkdir -p $experiment_outdir/kernelization/small/iso-kept-no-kernelization

            echo "Handle n: $n, nc: $nc"
            $builddir/./$bin  -n $n -nc $nc -kernelization-efficiency -remove-iso \
                    -output-path $experiment_outdir/kernelization/small/iso-rem/aggregate > $experiment_outdir/kernelization/small/iso-rem/out-exe_n"$n"_c"$nc".txt && echo "Done iso-rem" &
            
            $builddir/./$bin  -n $n -nc $nc -remove-iso \
                    -output-path $experiment_outdir/kernelization/small/iso-rem-no-kernelization/aggregate > $experiment_outdir/kernelization/small/iso-rem-no-kernelization/out-exe_n"$n"_c"$nc".txt && echo "Done iso-rem-no-kernelization" &
            if [ $n -lt 7 ] ; then
                    dorough=""
            else
                    dorough="-dorough"
            fi

            $builddir/./$bin  -n $n -nc $nc -kernelization-efficiency $dorough \
                    -output-path $experiment_outdir/kernelization/small/iso-kept/aggregate > $experiment_outdir/kernelization/small/iso-kept/out-exe_n"$n"_c"$nc".txt && echo "Done iso-kept" &

            $builddir/./$bin  -n $n -nc $nc $dorough \
                    -output-path $experiment_outdir/kernelization/small/iso-kept-no-kernelization/aggregate > $experiment_outdir/kernelization/small/iso-kept-no-kernelization/out-exe_n"$n"_c"$nc".txt && echo "Done iso-kept-no-kernelization" &

            echo "... wait"
            wait
        done
    done
}

func_localize