# Simple script to iterate through all the runs in ANITA-4 to check for missing folders / files for raw and rootified data
#!/bin/sh

FIRSTRUN=1
LASTRUN=367

bold=$(tput bold)
normal=$(tput sgr0)

for ((n=${FIRSTRUN};n<=${LASTRUN};n++)); do
    
    if [ $n == ${FIRSTRUN} ]; then
	echo "${bold}Starting to check the ANITA-4 rawdata set...${normal}"
    fi

    ### Re-comment one of the scripts if you want to run it
    ./rawFolderChecker.sh $n # which raw folders are missing?
    ./rootFileChecker.sh $n # which rootified files are missing?

    if [ $n == ${LASTRUN} ]; then
	echo "${bold}ANITA-4 data set checked"
    fi
    
done
