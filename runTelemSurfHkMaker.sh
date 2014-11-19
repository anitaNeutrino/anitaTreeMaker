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

echo "Starting SURF Hk File"
SURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/s*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${SURF_FILE_LIST}
  fi
done

#cat ${SURF_FILE_LIST}

if  test `cat ${SURF_FILE_LIST} | wc -l` -gt 0 ; then
    SURF_ROOT_FILE=${ROOT_RUN_DIR}/surfHkFile${RUN}.root
    ./makeSurfHkTree ${SURF_FILE_LIST} ${SURF_ROOT_FILE}
    rm ${SURF_FILE_LIST}
    echo "Done SURF Hk File"
else
    rm ${SURF_FILE_LIST}
    echo "No SURF Hk Files"
fi
