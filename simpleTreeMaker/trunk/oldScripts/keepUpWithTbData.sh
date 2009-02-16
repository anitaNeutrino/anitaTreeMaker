#/bin/bash

cd /home/anita/simpleTreeMaker

BASE_DIR=/TBdata/anita/antarctica08
RAW_DATA_DIR=${BASE_DIR}/raw
ROOT_DATA_DIR=${BASE_DIR}/root

while [ 1 ] ; do
    for runDir in ${RAW_DATA_DIR}/run* ; do
#	echo $runDir
	runNum=${runDir##*/run}
#	echo $runNum
	
	previousRunNum=$((runNum-1))
#	echo $previousRunNum

	rawDir=${RAW_DATA_DIR}/run${previousRunNum}
	rootDir=${ROOT_DATA_DIR}/run${previousRunNum}

	if [ -d "$rawDir" ]
	    then
	    if [ -d "$rootDir" ] 
		then
		a=1
#	    echo "$rootDir exists"
	    else
		echo "Will delete .dat files to avoid double counting"
		for datFile in ${runDir}/event/ev*/ev*/*.dat; do
		#echo $datFile
		    rm -f $datFile
		done
		for datFile in ${runDir}/house/gps/*/*/*/*/*.dat; do
		#echo $datFile
		    rm -f $datFile
		done
		for datFile in ${runDir}/house/hk/*/*/*/*.dat; do
		#echo $datFile
		    rm -f $datFile
		done
		for datFile in ${runDir}/house/monitor/*/*/*.dat; do
		#echo $datFile
		    rm -f $datFile
		done
		for datFile in ${runDir}/house/surfhk/*/*/*.dat; do
		#echo $datFile
		    rm -f $datFile
		done
		for datFile in ${runDir}/house/turfhk/*/*/*.dat; do
		#echo $datFile
		    rm -f $datFile
		done
		echo "Will make root files"
	    ./runTBdataFileMaker.sh $previousRunNum
	    ./quickCheckForErrors ${ROOT_DATA_DIR} $previousRunNum > /dev/null < /dev/null 2>&1 &
	    fi	    
	fi
    done    
    sleep 30
done
