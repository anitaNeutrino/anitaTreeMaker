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

echo "Starting Averaged SURF Hk File"
AVGSURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/a*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${AVGSURF_FILE_LIST}
  fi
done

#cat ${AVGSURF_FILE_LIST}

if  test `cat ${AVGSURF_FILE_LIST} | wc -l` -gt 0 ; then
    AVGSURF_ROOT_FILE=${ROOT_RUN_DIR}/avgSurfHkFile${RUN}.root
    makeAveragedSurfHkTree ${AVGSURF_FILE_LIST} ${AVGSURF_ROOT_FILE}
    rm ${AVGSURF_FILE_LIST}
    echo "Done Averaged SURF Hk File"
else
    rm ${AVGSURF_FILE_LIST}
    echo "No Averaged SURF Hk Files"
fi
