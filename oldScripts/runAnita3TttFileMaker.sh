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





echo "Starting TTT  File"
TTT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/all/ttt/*/*/ttt*.gz
  do
  if [[ -f $file ]]; then
      echo $file >> ${TTT_FILE_LIST}
  fi
done

if  test `cat ${TTT_FILE_LIST} | wc -l` -gt 0 ; then
    TTT_ROOT_FILE=${ROOT_RUN_DIR}/tttFile${RUN}.root
    ./makeGpsTttTree ${TTT_FILE_LIST} ${TTT_ROOT_FILE}
    rm ${TTT_FILE_LIST}
    echo "Done TTT  File"
else
    rm ${TTT_FILE_LIST}
    echo "No TTT  Files"
fi


