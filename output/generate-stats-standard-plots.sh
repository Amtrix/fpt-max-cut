#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    outdir_ifnot_given=$cwd/experiments-plots/standard-stats
    source $cwd/generate-bootstrap.sh

    declare -a arr=("512" "512w" "2048" "2048w" "8000" "8000w")
    declare -a arrl=("0.1" "0.2" "0.3" "0.4" "0.5")

    mkdir -p $outdir/kagen

    for i in "${arr[@]}"
    do
        set -o xtrace
        Rscript $cwd/R-kagen-standard.r --file $kagendata_standard/n$i/out --out $outdir/kagen/plot-n$i.pdf
        set +o xtrace

        for j in "${arrl[@]}"
        do
            local noperiodj="${j//./}"
            set -o xtrace
            Rscript $cwd/R-kagen-standard.r --file $kagendata_standard/n$i/out --out $outdir/kagen/plot-n$i-$noperiodj.pdf --loess $j
            Rscript $cwd/R-kagen-standard.r --file $kagendata_standard/n$i/out --out $outdir/kagen/plot-n$i-$noperiodj-nop.pdf --loess $j --nopoints
            set +o xtrace
        done
    done
}

func_localize