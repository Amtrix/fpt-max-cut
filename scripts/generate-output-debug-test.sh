#!/bin/bash
cd "${0%/*}"
cd ../build

make benchmark-debug

./benchmark-debug -action "kernelization" -sample-kagen 20 -benchmark-output ../data/output/kernelization/out.test > ../data/output/kernelization/out.test-exe -num-edges-lo 150 -num-edges-hi 350 -num-nodes 150 -locsearch-iterations 100 -total-allowed-time 5
cd ../data/output/kernelization
Rscript plotting-compare.r --file out.test --out plot-cmp-out.test.pdf
cd ../../../build