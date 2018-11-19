#!/bin/bash

cd "${0%/*}"
cd ../../build
cmake ../

#selected_build=benchmark
read -p "Select the build (benchmark or benchmark-debug): " selected_build

make $selected_build
