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

cd ${ANITA_UTIL_INSTALL_DIR}/bin

echo "Starting Header File"
HEAD_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/event/*/ev?/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev?????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done

cat ${HEAD_FILE_LIST}

if  test `cat ${HEAD_FILE_LIST} | wc -l` -gt 0 ; then
    HEAD_ROOT_FILE=${ROOT_RUN_DIR}/headFile${RUN}.root
    ./makeRawHeadTree ${HEAD_FILE_LIST} ${HEAD_ROOT_FILE}
    rm ${HEAD_FILE_LIST}
    DONE_HEAD_FILE=true
    ./makeA4TimedHeaderFile $RUN
    echo "Done Header File"
else
    rm ${HEAD_FILE_LIST}
    DONE_HEAD_FILE=false
    echo "No header files"
fi

