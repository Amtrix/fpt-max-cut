#!/bin/bash

mydir=`dirname $0`
echo $mydir
rsync -avzh dferizovic@i10login.iti.kit.edu:mthesis/data/output/experiments $mydir/remote/

