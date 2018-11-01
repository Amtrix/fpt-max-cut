#!/bin/bash
cd "${0%/*}"
cd ../build

make benchmark
./benchmark -action "kernelization" -sample-kagen 1 -benchmark-output ../data/output/kernelization/out1 > ../data/output/kernelization/out1-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000
./benchmark -action "kernelization" -sample-kagen 5 -benchmark-output ../data/output/kernelization/out5 > ../data/output/kernelization/out5-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000
./benchmark -action "kernelization" -sample-kagen 20 -benchmark-output ../data/output/kernelization/out20 > ../data/output/kernelization/out20-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000
./benchmark -action "kernelization" -sample-kagen 50 -benchmark-output ../data/output/kernelization/out50 > ../data/output/kernelization/out50-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000
./benchmark -action "kernelization" -sample-kagen 200 -benchmark-output ../data/output/kernelization/out200 > ../data/output/kernelization/out200-exe -num-edges-lo 10000 -num-edges-hi 16000 -num-nodes 8000