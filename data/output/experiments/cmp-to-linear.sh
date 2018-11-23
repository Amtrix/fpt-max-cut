#/bin/bash

rm plots/*.pdf

Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot.pdf
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-01.pdf --loess 0.1
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-02.pdf --loess 0.2
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-03.pdf --loess 0.3
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-05.pdf --loess 0.5
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-07.pdf --loess 0.7

Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-01-nop.pdf --loess 0.1 --nopoints
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-02-nop.pdf --loess 0.2 --nopoints
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-03-nop.pdf --loess 0.3 --nopoints
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-05-nop.pdf --loess 0.5 --nopoints
Rscript cmp-to-linear.r --file kernelization/n2048/out --filelinear linear-kernel/n2048/out --out plots/plot-07-nop.pdf --loess 0.7 --nopoints