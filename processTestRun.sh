# simple script for rootifying selections of ANITA-4 data

#!/bin/sh

FIRSTRUN=50
LASTRUN=50

bold=$(tput bold)
normal=$(tput sgr0)

for ((n=${FIRSTRUN};n<=${LASTRUN};n++)); do
    
    if [ $n == ${FIRSTRUN} ]; then
	echo "${bold}Starting to processes the ANITA-4 data set..."
    fi    

    echo "${bold}Now on run $n${normal}"
    
    ### uncomment which selection of data you want to process    
    #./runEventFileMaker.sh $n # for events only
    #./runNonEventFileMaker.sh # house 
    #./runNonEventFileMakerWHead.sh $n # house + event headers
    #./runHeaderFileMaker.sh $n # headers only
    #./runHeadFileMakerANITA4.sh $n # includes new headers (timed)
    ./ANITA4rootifier.sh $n # full rootifier
#    if [ $n > 41 ]; then
	#./build/makeGpsEventTreeBothAdu5s $n
#    fi
    echo "${bold}Run $n complete! ${normal}"
    echo "<----------------------------------------->"

    if [ $n == ${LASTRUN} ]; then
	echo "${bold}ANITA-4 data processed"
    fi
    
done
