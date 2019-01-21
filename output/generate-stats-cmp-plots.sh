#/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    outdir_ifnot_given=$cwd/experiments-plots/cmp
    source $cwd/generate-bootstrap.sh

    mkdir -p $outdir/experiments_B

    declare -a arrl=("0.15" "0.2" "0.3" "0.5" "0.7")

    subdir="."

    set -o xtrace
    Rscript $cwd/R-kagen-cmp-standard-to-linear_fpt.r      --file $kagendata_standard/n2048/out  --filelinear $kagendata_linear_fpt/n2048/out  --out $outdir/plot.pdf
    Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048w/$subdir/out --fileB $kagendata_standard/n2048/out  --out $outdir/plotw.pdf
    set +o xtrace

    for i in "${arrl[@]}"
    do
        local noperiodi="${i//./}"
        set -o xtrace
        Rscript $cwd/R-kagen-cmp-standard-to-linear_fpt.r  --file $kagendata_standard/n2048/$subdir/out  --filelinear $kagendata_linear_fpt/n2048/out --out $outdir/plot-$noperiodi.pdf --loess $i
        Rscript $cwd/R-kagen-cmp-standard-to-linear_fpt.r  --file $kagendata_standard/n2048/$subdir/out  --filelinear $kagendata_linear_fpt/n2048/out --out $outdir/plot-$noperiodi-nop.pdf --loess $i --nopoints

        Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048w/$subdir/out --fileB $kagendata_standard/n2048/out --out $outdir/plotw-$noperiodi.pdf --loess $i
        Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048w/$subdir/out --fileB $kagendata_standard/n2048/out --out $outdir/plotw-$noperiodi-nop.pdf --loess $i --nopoints
        set +o xtrace
    done

    Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048/$subdir/out --fileB $experiments_B/$standard_exp_subdir/n2048/out \
                                                               --out $outdir/experiments_B/plot.pdf

    for i in "${arrl[@]}"
    do
        local noperiodi="${i//./}"
        set -o xtrace
        #unweighted with unweighted result
        Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048/$subdir/out --fileB $experiments_B/$standard_exp_subdir/n2048/out \
                                                               --out $outdir/experiments_B/plot-$noperiodi.pdf --loess $i

        Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048/$subdir/out --fileB $experiments_B/$standard_exp_subdir/n2048/out \
                                                               --out $outdir/experiments_B/plot-$noperiodi-nop.pdf --loess $i --nopoints

        #weighted with weighted result
        Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048w/$subdir/out --fileB $experiments_B/$standard_exp_subdir/n2048w/out \
                                                               --out $outdir/experiments_B/plotw-$noperiodi.pdf --loess $i

        Rscript $cwd/R-kagen-cmp-standard_A-to-standard_B.r    --fileA $kagendata_standard/n2048w/$subdir/out --fileB $experiments_B/$standard_exp_subdir/n2048w/out \
                                                               --out $outdir/experiments_B/plotw-$noperiodi-nop.pdf --loess $i --nopoints
        set +o xtrace
    done
}

func_localize