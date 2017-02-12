#!/bin/bash 

#SBATCH --job-name=rootifyAnita4
#SBATCH --output=./log/%j.log 
#SBATCH --time=08:00:00 
#SBATCH --account=kicp
#SBATCH --partition=kicp
#SBATCH --cpus-per-task=1

echo $@ 
source /home/cozzyd/anita/env.sh

srun sh anita4FlightFileMakerUChicago.sh $1

