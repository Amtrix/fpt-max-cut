#/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    outdir_ifnot_given=$cwd/experiments-plots/cmp
    source $cwd/generate-bootstrap.sh

    rm $cwd/experiments-plots/very-small/* -r
    mkdir -p $cwd/experiments-plots/very-small

    Rscript $cwd/R-very-small.r --file $cwd/experiments/kernelization/small/iso-rem/aggregate --out $cwd/experiments-plots/very-small/plot-iso-rem.pdf --mode "iso-rem"
    Rscript $cwd/R-very-small.r --file $cwd/experiments/kernelization/small/iso-kept/aggregate --out $cwd/experiments-plots/very-small/plot-iso-kept.pdf --mode "iso-kept"

}

func_localize