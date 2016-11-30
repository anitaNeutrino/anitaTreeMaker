#!/bin/bash
if [ "$3" = "" ]
then
   echo "usage: `basename $0` <run no> <raw run ir> <root run dir>" 1>&2
   exit 1
fi

RUN=$1
RAW_RUN_DIR=$2
ROOT_RUN_DIR=$3
EVENT_BASE_DIR=`dirname $ROOT_RUN_DIR`


if [[ -d $ROOT_RUN_DIR ]]; then
    echo "Output dir exists"
elif [[ -d $RAW_RUN_DIR  ]]; then
    mkdir ${ROOT_RUN_DIR}
else
    echo "$RAW_RUN_DIR doesn't exist what are we suppposed to rootify?"
    exit 0;
fi




echo "Starting Event Header File"
HEAD_FILE_LIST=`mktemp`
find ${RAW_RUN_DIR}/event -name "evhd*.gz" | sort -n > ${HEAD_FILE_LIST}
#cat $HEAD_FILE_LIST
if  test `cat ${HEAD_FILE_LIST} | wc -l` -gt 0 ; then
    HEAD_ROOT_FILE=${ROOT_RUN_DIR}/eventHeadFile${RUN}.root
    makeEventHeadTree ${HEAD_FILE_LIST} ${HEAD_ROOT_FILE}
    rm ${HEAD_FILE_LIST}
    DONE_HEAD_FILE=true
    echo "Done Header File"
else
    rm ${HEAD_FILE_LIST}
    DONE_HEAD_FILE=false
    echo "No header files"
fi

echo "Starting Event File"
EVENT_FILE_LIST=`mktemp`
find ${RAW_RUN_DIR}/event -name "psev*.gz" | sort -n> ${EVENT_FILE_LIST}
if  test `cat ${EVENT_FILE_LIST} | wc -l` -gt 0 ; then
    updateEventRunTree ${EVENT_FILE_LIST} ${EVENT_BASE_DIR}
    rm ${EVENT_FILE_LIST}
    echo "Done Event File"
else
    rm ${EVENT_FILE_LIST}
    echo "No event files"
fi
 
