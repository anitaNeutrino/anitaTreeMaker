#!/bin/sh

FIRSTRUN=1
LASTRUN=367

bold=$(tput bold)
normal=$(tput sgr0)

for ((n=${FIRSTRUN};n<=${LASTRUN};n++)); do
    
    if [ $n == ${FIRSTRUN} ]; then
	echo "${bold}Starting to check the ANITA-4 rawdata set...${normal}"
    fi    
    ./FolderScript.sh $n

    if [ $n == ${LASTRUN} ]; then
	echo "${bold}ANITA-4 data set checked"
    fi
    
done
