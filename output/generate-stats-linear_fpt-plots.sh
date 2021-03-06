#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    outdir_ifnot_given=$cwd/experiments-plots/linear_fpt-stats
    source $cwd/generate-bootstrap.sh

    declare -a arr=("512" "2048")
    declare -a arrl=("0.1" "0.2" "0.3" "0.4" "0.5")

    mkdir -p $outdir/kagen

    for i in "${arr[@]}"
    do
        set -o xtrace
        Rscript $cwd/R-kagen-linear_fpt.r --file $kagendata_linear_fpt/n$i/out --out $outdir/kagen/plot-n$i.pdf
        set +o xtrace

        for j in "${arrl[@]}"
        do
            local noperiodj="${j//./}"
            set -o xtrace
            Rscript $cwd/R-kagen-linear_fpt.r --file $kagendata_linear_fpt/n$i/out --out $outdir/kagen/plot-n$i-$noperiodj.pdf --loess $j
            Rscript $cwd/R-kagen-linear_fpt.r --file $kagendata_linear_fpt/n$i/out --out $outdir/kagen/plot-n$i-$noperiodj-nop.pdf --loess $j --nopoints
            set +o xtrace
        done
    done
}

func_localize