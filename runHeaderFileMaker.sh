#/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi


RUN=$1
BASE_DIR=/unix/anita4/flight2016
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}
TREE_MAKER_DIR=/home/batten/anitaTreeMaker/build/ # Put your custom directory here to lead to your existing anitaTreeMaker/build

if [ ! -d "$TREE_MAKER_DIR" ]; then
    echo "TREE_MAKER_DIR ($TREE_MAKER_DIR) does not exist. Please check the directory leading to your anitaTreeMaker/dir setup exists within this file. Aborting."
    exit 0;
fi

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

echo "Rootifying non-event data..."
echo "Non-event data includes: calib, gps, gpu, hk, monitor, rtl, surfhk, tuff, turfhk,  aux files and well as header files"

cd ${ANITA_UTIL_INSTALL_DIR}/bin
#cat ${HEAD_FILE_LIST}

cd ${TREE_MAKER_DIR}

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
    echo "Done Header File"
else
    rm ${HEAD_FILE_LIST}
    DONE_HEAD_FILE=false
    echo "No header files"
fi


