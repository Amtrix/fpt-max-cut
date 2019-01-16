#!/bin/bash

func_localize() {
    local cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

    mkdir -p $cwd/remote-history/most-recent/
    rm $cwd/remote-history/most-recent/* -r
    rsync -avzh dferizovic@i10login.iti.kit.edu:mthesis/output/experiments $cwd/remote-history/most-recent/
}

func_localize

