#/bin/bash

rm plots/*.pdf

Rscript plotting.r --file real-world/out-maxcut_live-localsolver --out plots/plot-localsolver.pdf
Rscript plotting.r --file real-world/out-maxcut_live-localsolver --out plots/plot-localsolver-nopoints-loess.pdf --nopoints --loess 0.01
Rscript plotting.r --file real-world/out-maxcut_live-localsolver --out plots/plot-localsolver-loess.pdf --loess 0.01