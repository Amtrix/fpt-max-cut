#/bin/bash

set -o xtrace

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    g++ $cwd/cpp-latex_table-create-isolated.cpp -o $cwd/cpp-latex_table-create-isolated.o

    declare -a types=("RHG" "RGG2D" "RGG3D" "GNM" "BA")

    mkdir -p $cwd/experiments-plots/cmp/isolated
    for i in "${types[@]}"
    do
        $cwd/./cpp-latex_table-create-isolated.o $cwd/experiments $i
        Rscript $cwd/R-kagen-cmp-isolated.r -t $i -o $cwd/experiments-plots/cmp/isolated/$i.pdf
    done
}

func_localize