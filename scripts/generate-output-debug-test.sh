#!/bin/bash
cd "${0%/*}"
cd ../build

make benchmark-debug

./benchmark-debug -action "kernelization" -sample-kagen 10 -benchmark-output ../data/output/kernelization/out.test > ../data/output/kernelization/out.test-exe -num-edges-lo 50 -num-edges-hi 250 -num-nodes 50 -locsearch-iterations 200
cd ../data/output/kernelization
Rscript plotting-compare.r --file out.test --out plot-cmp-out.test.pdf
cd ../../../build