#/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    g++ $cwd/cpp-latex_table-create_isolated.cpp -o $cwd/cpp-latex_table-create_isolated.o
    g++ $cwd/cpp-latex_table-marked.cpp -o $cwd/cpp-latex_table-marked.o
    g++ $cwd/cpp-latex_table-create_solvers.cpp -o $cwd/cpp-latex_table-create_solvers.o
    g++ $cwd/cpp-latex_table-create_biqmac.cpp -o $cwd/cpp-latex_table-create_biqmac.o
    g++ $cwd/cpp-latex_table-create_live.cpp -o $cwd/cpp-latex_table-create_live.o
}

func_localize