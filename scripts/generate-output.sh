#!/bin/bash
cd "${0%/*}"
cd ../build

make benchmark

declare -a arr=("1" "5" "20" "50")

for i in "${arr[@]}"
do
    ./benchmark -action "kernelization" -sample-kagen $i -benchmark-output ../data/output/kernelization/out$i > ../data/output/kernelization/out$i-exe -num-edges-lo 4000 -num-edges-hi 40000 -num-nodes 8000 -locsearch-iterations 200
    cd ../data/output/kernelization
    Rscript plotting-compare.r --file out$i --out plot-cmp-out$i.pdf
    Rscript plotting-immense.r --file out$i --out plot-out$i.pdf
    cd ../../../build
done

#./benchmark -action "kernelization" -sample-kagen 20 -benchmark-output ../data/output/kernelization/out20 > ../data/output/kernelization/out20-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000
#./benchmark -action "kernelization" -sample-kagen 50 -benchmark-output ../data/output/kernelization/out50 > ../data/output/kernelization/out50-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000
#./benchmark -action "kernelization" -sample-kagen 200 -benchmark-output ../data/output/kernelization/out200 > ../data/output/kernelization/out200-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000