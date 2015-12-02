#/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi

RUN=$1
BASE_DIR=/unix/anita3/flight1415
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





echo "Starting TURF Rate File"
TURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/turfhk/*/*/t*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${TURF_FILE_LIST}
  fi
done

if  test `cat ${TURF_FILE_LIST} | wc -l` -gt 0 ; then
    TURF_ROOT_FILE=${ROOT_RUN_DIR}/turfRateFile${RUN}.root
    ./makeTurfRateTree ${TURF_FILE_LIST} ${TURF_ROOT_FILE}
    rm ${TURF_FILE_LIST}
    echo "Done TURF Rate File"
else
    rm ${TURF_FILE_LIST}
    echo "No TURF Rate Files"
fi


