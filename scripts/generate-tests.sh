#!/bin/bash
cd "${0%/*}"
cd ../test-generators/KaGen


# SPARSE
./build/app/kagen -gen rhg -n 13 -gamma 2.4 -d  2 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d02.123
./build/app/kagen -gen rhg -n 13 -gamma 2.4 -d  4 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d04.123
./build/app/kagen -gen rhg -n 13 -gamma 2.4 -d  8 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d08.123
./build/app/kagen -gen rhg -n 13 -gamma 2.4 -d 16 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d16.123
./build/app/kagen -gen rhg -n 13 -gamma 2.4 -d 32 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d32.123
./build/app/kagen -gen rhg -n 13 -gamma 2.4 -d 64 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d64.123


./build/app/kagen -gen ba -n 13 -md  1 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d01.123
./build/app/kagen -gen ba -n 13 -md  2 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d02.123
./build/app/kagen -gen ba -n 13 -md  4 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d04.123
./build/app/kagen -gen ba -n 13 -md  8 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d08.123
./build/app/kagen -gen ba -n 13 -md 16 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d16.123

./build/app/kagen -gen gnm_undirected -n 13 -m 13 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m13.123
./build/app/kagen -gen gnm_undirected -n 13 -m 14 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m14.123
./build/app/kagen -gen gnm_undirected -n 13 -m 15 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m15.123
./build/app/kagen -gen gnm_undirected -n 13 -m 16 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m16.123
./build/app/kagen -gen gnm_undirected -n 13 -m 17 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m17.123

#./build/app/kagen -gen rgg_2d -n 13 -r 0.005 -seed 123 -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.005.123
./build/app/kagen -gen rgg_2d -n 13 -r 0.01 -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.01.123
./build/app/kagen -gen rgg_2d -n 13 -r 0.02 -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.02.123
./build/app/kagen -gen rgg_2d -n 13 -r 0.03 -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.03.123
./build/app/kagen -gen rgg_2d -n 13 -r 0.04 -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.04.123

./build/app/kagen -gen rgg_3d -n 13 -r 0.03  -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.030.123
./build/app/kagen -gen rgg_3d -n 13 -r 0.045 -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.045.123
./build/app/kagen -gen rgg_3d -n 13 -r 0.06  -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.060.123
./build/app/kagen -gen rgg_3d -n 13 -r 0.075 -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.075.123
./build/app/kagen -gen rgg_3d -n 13 -r 0.09  -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.090.123
./build/app/kagen -gen rgg_3d -n 13 -r 0.105 -seed 123  -output ../../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.105.123

cd ../../build

./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d02.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d04.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d08.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d16.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d32.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rhg-n13_g2.4_d64.123


./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d01.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d02.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d04.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d08.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/ba-n13_d16.123


./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m13.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m14.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m15.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m16.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/gnm-n13_m17.123

#./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.005.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.01.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.02.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.03.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_2d-n13_r0.04.123

./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.030.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.045.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.060.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.075.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.090.123
./remove-double-edges -data ../data/thesis-tests/kagen-large-sparse/tests/rgg_3d-n13_r0.105.123
