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


echo "Starting Summed TURF Rate File"
SUMTURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/turfhk/*/*/s*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${SUMTURF_FILE_LIST}
  fi
done

if  test `cat ${SUMTURF_FILE_LIST} | wc -l` -gt 0 ; then
    SUMTURF_ROOT_FILE=${ROOT_RUN_DIR}/sumTurfRateFile${RUN}.root
    ./makeSummedTurfRateTree ${SUMTURF_FILE_LIST} ${SUMTURF_ROOT_FILE}
    rm ${SUMTURF_FILE_LIST}
    echo "Done SUMTURF Rate File"
else
    rm ${SUMTURF_FILE_LIST}
    echo "No Summed TURF Rate Files"
fi
