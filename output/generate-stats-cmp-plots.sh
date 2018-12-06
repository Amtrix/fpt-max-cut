#/bin/bash

cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

outdir_ifnot_given=$cwd/experiments-plots/cmp
source $cwd/generate-bootstrap.sh

declare -a arrl=("0.15" "0.2" "0.3" "0.5" "0.7")

Rscript $cwd/R-kagen-cmp-standard-to-linear_fpt.r      --file $kagendata_standard/n2048/out  --filelinear $kagendata_linear_fpt/n2048/out  --out $outdir/plot.pdf
Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048w/out --fileB $kagendata_standard/n2048/out  --out $outdir/plotw.pdf

for i in "${arrl[@]}"
do
    noperiodi="${i//./}"
    Rscript $cwd/R-kagen-cmp-standard-to-linear_fpt.r  --file $kagendata_standard/n2048/out  --filelinear $kagendata_linear_fpt/n2048/out --out $outdir/plot-$noperiodi.pdf --loess $i
    Rscript $cwd/R-kagen-cmp-standard-to-linear_fpt.r  --file $kagendata_standard/n2048/out  --filelinear $kagendata_linear_fpt/n2048/out --out $outdir/plot-$noperiodi-nop.pdf --loess $i --nopoints

    Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048w/out --fileB $kagendata_standard/n2048/out --out $outdir/plotw-$noperiodi.pdf --loess $i
    Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048w/out --fileB $kagendata_standard/n2048/out --out $outdir/plotw-$noperiodi-nop.pdf --loess $i --nopoints
done
