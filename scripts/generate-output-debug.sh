#!/bin/bash
cd "${0%/*}"
cd ../build

make benchmark-debug
./benchmark-debug -action "kernelization" -sample-kagen 1 -benchmark-output ../data/output/kernelization/out1 > ../data/output/kernelization/out1-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000 -locsearch-iterations 200
cd ../data/output/kernelization
Rscript plotting-compare.r --file out1 --out plot-cmp-out1.pdf
cd ../../../build