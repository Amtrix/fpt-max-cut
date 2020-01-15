#!/bin/bash

git submodule update --init --recursive
cd solvers/MQLIb
make
cd ../../
mkdir build
cd build
cmake ../
make benchmark
