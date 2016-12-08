#!/bin/bash

inputFolder=$1;
firstRun=$2;
lastRun=$3;

cd ~/Code/anitaBuildTool/build/components/anitaTreeMaker

for irun in `seq $firstRun $lastRun`; do
    echo Reproducing eventHeadFile and eventFile for run $irun
    ./fixTelemHeadEventTrees $inputFolder $irun
done
