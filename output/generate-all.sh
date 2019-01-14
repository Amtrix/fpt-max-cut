#/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    source $cwd/generate-stats-standard-plots.sh
    source $cwd/generate-stats-linear_fpt-plots.sh
    source $cwd/generate-stats-cmp-plots.sh
    source $cwd/generate-stats-cmp-isolated.sh
}

func_localize