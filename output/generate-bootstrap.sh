#/bin/bash

cwdbootplots="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

standard_exp_subdir=kernelization

experiments=$cwdbootplots/experiments
experiments_B=$cwdbootplots/previous/experiments-presentation-official-all

kagendata_standard=$experiments/$standard_exp_subdir
kagendata_linear_fpt=$experiments/linear-kernel
timeana_realworld=$experiments/solvers/real-world

#if [ -z ${outdir+x} ] ; then
    outdir=$outdir_ifnot_given
    mkdir -p $outdir
#fi

if [ ! -d "$outdir" ]; then
    echo "Provided output path does not exist in outdir."
    exit 1
fi

rm $outdir/* -r