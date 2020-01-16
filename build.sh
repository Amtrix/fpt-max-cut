#!/bin/bash

rm -rf build
git submodule update --init --recursive
cd solvers/MQLib/
make
cd ../../
mkdir build
cd build
cmake ../
make benchmark
