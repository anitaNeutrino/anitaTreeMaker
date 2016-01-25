#/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi

RUN=$1
BASE_DIR=/unix/anita3/flight1415
#BASE_DIR=/Users/rjn/anita/anita3/data/
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

if [[ -d $ROOT_RUN_DIR ]]; then
    echo "Output dir exists"
elif [[ -d $RAW_RUN_DIR  ]]; then
    mkdir ${ROOT_RUN_DIR}
else
    echo "$RAW_RUN_DIR doesn't exist what are we suppposed to rootify?"
    exit 0;
fi

echo "Using $RAW_RUN_DIR"
ls ${RAW_RUN_DIR}





HEAD_ROOT_FILE=${ROOT_RUN_DIR}/headFile${RUN}.root
NEW_HEAD_ROOT_FILE=${ROOT_RUN_DIR}/timedHeadFile${RUN}.root
EPOCH_FILE=/unix/anita3/flight1415/epochFiles/newEpochTree${RUN}.root
./fixTriggerTimeAnita3 $HEAD_ROOT_FILE $EPOCH_FILE $NEW_HEAD_ROOT_FILE

