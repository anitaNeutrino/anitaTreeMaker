#!/bin/bash

firstRun=$1;
lastRun=$2

for irun in `seq $firstRun $lastRun`; do
    echo Reproducing headTree for run $irun
    ./runLDBHeadFileMaker.sh $irun
done
