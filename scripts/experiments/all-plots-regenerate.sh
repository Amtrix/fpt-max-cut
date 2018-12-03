#!/bin/bash

cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
outdir=$cwd/../../data/output/plots/cmp

source $cwd/../../data/output/experiments/generate-cmp-plots.sh