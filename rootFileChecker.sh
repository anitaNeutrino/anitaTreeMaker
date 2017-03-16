### Simple script to check for missing root files in the ANITA-4 dataset

#/bin/bash
if [ "$1" = "" ]
then
    echo "usage: `basename $0` <run no>" 1>&2
    exit 1
fi

###

RUN=$1
BASE_DIR=$ANITA_4_BASE_DIR
RUN_BASE_DIR=${BASE_DIR}/run${RUN}

if [ ! -d "$BASE_DIR" ]; then
    echo "$BASE_DIR ($BASE_DIR) does not exist. Which root files are we supposed to check? Please include your BASE_DIR  file. Aborting."
    exit 0;
fi

if [ ! -d "$RUN_BASE_DIR" ]; then
    echo "The root files for run ${RUN} are missing!"
fi

# switch to run root files

cd ${RUN_BASE_DIR}

# Files to search
declare -a rootFiles=("aux" "avgSurfHk" "event" "gps" "gpu" "head" "hk" "monitor" "prettyHk" "rawScaler" "rtlSpectrum" "sshk" "sumTurfRate" "surfHk" "timedHead" "tuffStatus" "turfRate")
rootFileslength=${#rootFiles[@]}

for (( i=0; i<${rootFileslength}; i++ ));
do
    
    if [ ! -f "${rootFiles[$i]}File${RUN}.root" ];
    then ## if the main directory doesn't exist, tell us
	echo "${rootFiles[$i]} root file missing in run ${RUN}"
	
    fi
done
