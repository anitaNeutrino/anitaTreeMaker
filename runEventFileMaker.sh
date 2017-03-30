#/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi

RUN=$1
BASE_DIR=${ANITA4_BASE_DIR}
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}
TREE_MAKER_DIR=${TREE_DIR}/build


if [ -d $ROOT_RUN_DIR ]; then
    echo "Output dir exists"
elif [ -d $RAW_RUN_DIR  ]; then
    mkdir ${ROOT_RUN_DIR}
else
    echo "$RAW_RUN_DIR doesn't exist what are we suppposed to rootify?"
    exit 0;
fi

echo "Using $RAW_RUN_DIR"
ls ${RAW_RUN_DIR}

cd ${TREE_MAKER_DIR}

echo "Starting Event File"

EVENT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/event/*/ev?/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev?????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done

if  test `cat ${EVENT_FILE_LIST} | wc -l` -gt 0 ; then
    ./makeEventRunTree ${EVENT_FILE_LIST} ${EVENT_BASE_DIR}
    rm ${EVENT_FILE_LIST}
    echo "Done Event File"
else
    rm ${EVENT_FILE_LIST}
    echo "No event files"
fi
