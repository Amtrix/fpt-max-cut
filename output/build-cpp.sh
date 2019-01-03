#/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    g++ $cwd/cpp-latex_table-create-isolated.cpp -o $cwd/cpp-latex_table-create-isolated.o
    g++ $cwd/cpp-latex_table-marked.cpp -o $cwd/cpp-latex_table-marked.o
}

func_localize