#!/bin/bash

func_localize() {
    #just to make it unique, weird suffix added
    local cwdallmain576="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"


    source $cwdallmain576/bootstrap.sh

    #echo "#0 Running"
    #source $cwdallmain576/stats-kernelization-kagen-rhg.sh

    echo "#1 Running"
    source $cwdallmain576/stats-kernelization-kagen.sh

    #echo "#2 Running"
    #source $cwdallmain576/stats-linear-kernel-kagen.sh

    #echo "#3 Running"
    #source $cwd/timekernelization-kagen-n100k-500k-sparse.sh

    #echo "#4 Solvers"
    #source $cwdallmain576/solvers-realworld.sh

    echo "#x Plotting..."
    source $cwdallmain576/all-plots-regenerate.sh

    echo "Done!"

    rm $experiment_outdir/gitcommit.info
    touch $experiment_outdir/gitcommit.info
    git rev-parse HEAD > $experiment_outdir/gitcommit.info
    echo "\n" >> $experiment_outdir/gitcommit.info
    git rev-parse --short HEAD >> $experiment_outdir/gitcommit.info
}

func_localize