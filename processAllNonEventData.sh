#!/bin/sh

FIRSTRUN=1
LASTRUN=367

bold=$(tput bold)
normal=$(tput sgr0)

for ((n=${FIRSTRUN};n<=${LASTRUN};n++)); do
    
    if [ $n == ${FIRSTRUN} ]; then
	echo "${bold}Starting to processes the ANITA-4 non-event data set..."
    fi    

    echo "${bold}Now on run $n${normal}"
    ./runNonEventFileMaker.sh $n
    echo "${bold}Run $n complete! ${normal}"
    echo "<----------------------------------------->"

    if [ $n == ${LASTRUN} ]; then
	echo "${bold}ANITA-4 non-event data processed"
    fi
    
done
