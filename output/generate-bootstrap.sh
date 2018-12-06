#/bin/bash

cwdboot="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

kagendata_standard=$cwdboot/experiments/kernelization
kagendata_linear_fpt=$cwdboot/experiments/linear-kernel
timeana_realworld=$cwdboot/experiments/solvers/real-world

#if [ -z ${outdir+x} ] ; then
    outdir=$outdir_ifnot_given
    mkdir -p $outdir
#fi

if [ ! -d "$outdir" ]; then
    echo "Provided output path does not exist in outdir."
    exit 1
fi

rm $outdir/* -r