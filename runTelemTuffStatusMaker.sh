#/bin/bash
if [ "$3" = "" ]
then
   echo "usage: `basename $0` <run no> <raw run dir> <root run dir>" 1>&2
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


echo "Starting TUFF STATUS File"
TUFF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/tuff/*/*/tuff*
  do
  if [[ -f $file ]]; then
      echo $file >> ${TUFF_FILE_LIST}
  fi
done

if  test `cat ${TUFF_FILE_LIST} | wc -l` -gt 0 ; then
    TUFF_ROOT_FILE=${ROOT_RUN_DIR}/tuffStatusFile${RUN}.root
    makeTuffStatusTree ${TUFF_FILE_LIST} ${TUFF_ROOT_FILE}
    rm ${TUFF_FILE_LIST}
    echo "Done TUFF File"
else
    rm ${TUFF_FILE_LIST}
    echo "No Tuff Files"
fi
