#!/bin/bash
cd "${0%/*}"
cd ../test-generators/KaGen



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
