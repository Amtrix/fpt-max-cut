#/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    outdir_ifnot_given=$cwd/experiments-plots/solvers
    source $cwd/generate-bootstrap.sh

    mkdir -p $outdir/real-world

    set -o xtrace
    Rscript $cwd/R-solvers-over-time.r --file $timeana_realworld/out-maxcut_live-localsolver --out $outdir/real-world/plot-localsolver.pdf
    Rscript $cwd/R-solvers-over-time.r --file $timeana_realworld/out-maxcut_live-localsolver --out $outdir/real-world/plot-localsolver-nopoints-loess.pdf --nopoints --loess 0.01
    Rscript $cwd/R-solvers-over-time.r --file $timeana_realworld/out-maxcut_live-localsolver --out $outdir/real-world/plot-localsolver-loess.pdf --loess 0.01
    set +o xtrace
}

func_localize