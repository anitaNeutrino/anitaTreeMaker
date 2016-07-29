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

echo "Starting Other File"
OTHER_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/monitor/*/*/other*.dat*; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${OTHER_FILE_LIST}
  fi
done

if  test `cat ${OTHER_FILE_LIST} | wc -l` -gt 0 ; then
    OTHER_ROOT_FILE=${ROOT_RUN_DIR}/monitorFile${RUN}.root
    ./makeOtherTree ${OTHER_FILE_LIST} ${OTHER_ROOT_FILE}
    rm ${OTHER_FILE_LIST}
    echo "Done Other Rate File"
else
    rm ${OTHER_FILE_LIST}
    echo "No Other Rate Files"
fi
