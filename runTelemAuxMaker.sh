#!/bin/bash
if [ "$3" = "" ]
then
   echo "usage: `basename $0` <run no> <raw run ir> <root run dir>" 1>&2
   exit 1
fi

RUN=$1
RAW_RUN_DIR=$2
ROOT_RUN_DIR=$3


if [[ -d $ROOT_RUN_DIR ]]; then
    echo "Output dir exists"
elif [[ -d $RAW_RUN_DIR  ]]; then
    mkdir ${ROOT_RUN_DIR}
else
    echo "$RAW_RUN_DIR doesn't exist what are we suppposed to rootify?"
    exit 0;
fi

cd $ANITA_TREE_MAKER_DIR


echo "Starting Auxiliary File"
ACQD_START_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/start/a*; 
do
    if [[ -f $file ]]; then
	echo $file >> ${ACQD_START_FILE_LIST}
    fi
done

GPSD_START_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/start/g*; 
do
    if [[ -f $file ]]; then
	echo $file >> ${GPSD_START_FILE_LIST}
    fi
done

LOGWATCHD_START_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/start/l*; 
do
    if [[ -f $file ]]; then
	echo $file >> ${LOGWATCHD_START_FILE_LIST}
    fi
done


CMD_ECHO_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/cmd/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${CMD_ECHO_FILE_LIST}
    fi
done

AUX_ROOT_FILE=${ROOT_RUN_DIR}/auxFile${RUN}.root
./makeAuxiliaryTree ${ACQD_START_FILE_LIST} ${GPSD_START_FILE_LIST} ${LOGWATCHD_START_FILE_LIST} ${CMD_ECHO_FILE_LIST} ${AUX_ROOT_FILE}
rm ${ACQD_START_FILE_LIST} ${GPSD_START_FILE_LIST} ${LOGWATCHD_START_FILE_LIST} ${CMD_ECHO_FILE_LIST}
echo "Done Auxiliary File"

