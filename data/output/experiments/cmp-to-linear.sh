#/bin/bash

rm plots/*.pdf

Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot.pdf
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-015.pdf --loess 0.15
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-02.pdf --loess 0.2
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-03.pdf --loess 0.3
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-05.pdf --loess 0.5
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-07.pdf --loess 0.7

Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-015-nop.pdf --loess 0.15 --nopoints
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-02-nop.pdf --loess 0.2 --nopoints
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-03-nop.pdf --loess 0.3 --nopoints
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-05-nop.pdf --loess 0.5 --nopoints
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-07-nop.pdf --loess 0.7 --nopoints


Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw.pdf
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-015.pdf --loess 0.15
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-02.pdf --loess 0.2
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-03.pdf --loess 0.3
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-05.pdf --loess 0.5
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-07.pdf --loess 0.7

Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-015-nop.pdf --loess 0.15 --nopoints
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-02-nop.pdf --loess 0.2 --nopoints
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-03-nop.pdf --loess 0.3 --nopoints
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-05-nop.pdf --loess 0.5 --nopoints
Rscript cmp-to-weighted.r --file kernelization/n2048w/out --filelinear kernelization/n2048/out --out plots/plotw-07-nop.pdf --loess 0.7 --nopoints