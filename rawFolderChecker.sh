### Simple script to check for missing raw directories in the ANITA-4 dataset

#/bin/bash
if [ "$1" = "" ]
then
    echo "usage: `basename $0` <run no>" 1>&2
    exit 1
fi

###

RUN=$1
BASE_DIR=$ANITA_4_BASE_DIR
START_DIR=${BASE_DIR}/run${RUN}/start
EVENT_DIR=${BASE_DIR}/run${RUN}/event
HOUSE_DIR=${BASE_DIR}/run${RUN}/house

if [ ! -d "$BASE_DIR" ]; then
    echo "BASE_DIR ($BASE_DIR) does not exist. Which raw file repositories are we supposed to check? Please include your BASE_DIR name in this file. Aborting."
    exit 0;
fi

if [ ! -d "$EVENT_DIR" ]; then
    echo "The event directory appears to be missing for run ${RUN}."
fi

if [ ! -d "$HOUSE_DIR" ]; then
    echo "The house directory appears to be missing for run ${RUN}."
fi

cd ${HOUSE_DIR} # Check all main dirs in house

# Dirs to search
declare -a MainDir=("calib" "gps" "gpu" "hk" "monitor" "rtl" "surfhk" "tuff" "turfhk")
MainDirlength=${#MainDir[@]}

# Subdirs to search
declare -a gpsSubDir=("adu5a" "adu5b" "all" "g12")
gpsSubDirlength=${#gpsSubDir[@]}

declare -a hkSubDir=("cal" "raw")
hkSubDirlength=${#hkSubDir[@]}
### Start the search --->

for (( i=0; i<${MainDirlength}; i++ ));
do
    
    if [ ! -d "${MainDir[$i]}" ];
    then ## if the main directory doesn't exist, tell us
	echo "${MainDir[$i]} raw dir missing in run ${RUN}"

    else ## if the directory does exist, search the subdirectories 
	if [ $i == 1 ] ## gps
	then
	    cd ${MainDir[$i]}
	    for (( j=0; j<${gpsSubDirlength}; j++ ));
	    do
		if [ ! -d "${gpsSubDir[$j]}" ];
		then ## if the sub directory doesn't exist, tell us
		    echo "${MainDir[$i]} main dir: ${gpsSubDir[$j]} raw sub dir missing in run ${RUN}"
		fi
	    done
	    cd ..
	fi
	
	if [ $i == 3 ] ## hk
	then
	    cd ${MainDir[$i]}
	    for (( j=0; j<${hkSubDirlength}; j++ ));
	    do
		if [ ! -d "${hkSubDir[$j]}" ];
		then ## if the sub directory doesn't exist, tell us
		    echo "${MainDir[$i]} main dir: ${hkSubDir[$j]} raw sub dir missing in run ${RUN}"
		fi
	    done
	    cd ..
	fi
    fi
done
