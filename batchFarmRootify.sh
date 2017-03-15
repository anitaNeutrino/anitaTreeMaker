# Quickly make ANITA-4 root files w/ batch farm
#!/bin/bash

FIRSTRUN=287
LASTRUN=367

bold=$(tput bold)
normal=$(tput sgr0)

for ((n=${FIRSTRUN};n<=${LASTRUN};n++)); do

    cd /home/batten/anitaTreeMaker/sendBatch
    
    cp runEventFileMaker.sh runEventFileMaker$n.sh
    sed -i "s/^RUN=.*/RUN=$n/" runEventFileMaker$n.sh
    qsub -q long -M luke.batten.15@ucl.ac.uk runEventFileMaker$n.sh

    #./runEventFileMaker$n.sh  # for events onlycd ..
    #./runNonEventFileMaker.sh # house 
    #./runNonEventFileMakerWHead.sh # house + event headers
    #./runHeaderFileMaker.sh # header
    echo "${bold}Run $n submitted! ${normal}"
    echo "<----------------------------------------->"

    if [ $n == ${LASTRUN} ]; then
	echo "${bold}ANITA-4 data processed"
    fi
    
done
