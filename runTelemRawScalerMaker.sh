#/bin/bash
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



echo "Starting SURF Raw Scaler File"
SCALER_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/raw*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${SCALER_FILE_LIST}
  fi
done

#cat ${SCALER_FILE_LIST}

if  test `cat ${SCALER_FILE_LIST} | wc -l` -gt 0 ; then
    SCALER_ROOT_FILE=${ROOT_RUN_DIR}/rawScalerFile${RUN}.root
    ./makeRawScalerTree ${SCALER_FILE_LIST} ${SCALER_ROOT_FILE}
    rm ${SCALER_FILE_LIST}
    echo "Done Raw Scaler File"
else
    rm ${SCALER_FILE_LIST}
    echo "No Raw Scaler Files"
fi
