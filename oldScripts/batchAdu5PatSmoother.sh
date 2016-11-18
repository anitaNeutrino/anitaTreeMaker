#!/bin/sh

# UChicago batch submitter for makeGpsEventTreeBothAdu5s
# Obviously only works at KICP on Midway cluster...
# -Cosmin 


mkdir -p log
for run in `seq $1 $2`; do
    RUN_DIR="$ANITA_ROOT_DATA/run$run"
    if [[ -d $RUN_DIR ]] ; then
      echo "Got $RUN_DIR"
      srun -A kicp -p kicp -o log/$run.out ./makeGpsEventTreeBothAdu5s $run  &> log/srun.log & 
      echo submitted $run
      sleep 5  # rate limit submissions to avoid limits 
    else
	echo "Missing $RUN_DIR"
    fi
done
