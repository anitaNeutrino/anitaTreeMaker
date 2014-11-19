#/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi

RUN=$1
BASE_DIR=/anitaStorage/antarctica14
RAW_RUN_DIR=${BASE_DIR}/telem/raw/run${RUN}
EVENT_BASE_DIR=${BASE_DIR}/telem/root
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


./runTelemHeaderMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemEventMaker.s $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemTurfRateMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemSurfHkMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemSumTurfRateMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemAvgSurfHkMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemMonitorMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemOtherMonitorMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemHkMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemGpsMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR
./runTelemAuxMaker.sh $RUN $RAW_RUN_DIR $ROOT_RUN_DIR



