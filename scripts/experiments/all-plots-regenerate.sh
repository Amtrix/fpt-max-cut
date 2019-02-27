#!/bin/bash

cwdmain="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

source $cwdmain/../../output/generate-solvers-plots.sh
source $cwdmain/../../output/generate-stats-cmp-plots.sh
source $cwdmain/../../output/generate-stats-linear_fpt-plots.sh
source $cwdmain/../../output/generate-stats-standard-plots.sh

