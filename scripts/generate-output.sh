#!/bin/bash
cd "${0%/*}"
cd ../build

make benchmark
./benchmark -action "kernelization" -sample-kagen 1 -benchmark-output ../data/output/kernelization/out1 > ../data/output/kernelization/out1-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000 -locsearch-iterations 200
cd ../data/output/kernelization
Rscript plotting-compare.r --file out1 --out plot-cmp-out1.pdf
cd ../../../build

./benchmark -action "kernelization" -sample-kagen 5 -benchmark-output ../data/output/kernelization/out5 > ../data/output/kernelization/out5-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000 -locsearch-iterations 200
cd ../data/output/kernelization
Rscript plotting-compare.r --file out5 --out plot-cmp-out5.pdf
cd ../../../build

./benchmark -action "kernelization" -sample-kagen 20 -benchmark-output ../data/output/kernelization/out20 > ../data/output/kernelization/out20-exe -num-edges-lo 8000 -num-edges-hi 24000 -num-nodes 8000 -locsearch-iterations 200
cd ../data/output/kernelization
Rscript plotting-compare.r --file out20 --out plot-cmp-out20.pdf
cd ../../../build

./benchmark -action "kernelization" -sample-kagen 50 -benchmark-output ../data/output/kernelization/out50 > ../data/output/kernelization/out50-exe -num-edges-lo 8000 -num-edges-hi 30000 -num-nodes 8000 -locsearch-iterations 200
cd ../data/output/kernelization
Rscript plotting-compare.r --file out50 --out plot-cmp-out50.pdf
cd ../../../build

#./benchmark -action "kernelization" -sample-kagen 20 -benchmark-output ../data/output/kernelization/out20 > ../data/output/kernelization/out20-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000
#./benchmark -action "kernelization" -sample-kagen 50 -benchmark-output ../data/output/kernelization/out50 > ../data/output/kernelization/out50-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000
#./benchmark -action "kernelization" -sample-kagen 200 -benchmark-output ../data/output/kernelization/out200 > ../data/output/kernelization/out200-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000