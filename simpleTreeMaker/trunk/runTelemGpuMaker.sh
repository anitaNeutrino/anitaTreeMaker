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


echo "Starting Gpu File"
GPU_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gpu/*/*/gpu*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${GPU_FILE_LIST}
  fi
done

if  test `cat ${GPU_FILE_LIST} | wc -l` -gt 0 ; then
    GPU_ROOT_FILE=${ROOT_RUN_DIR}/gpuFile${RUN}.root
    ./makeGpuTree ${GPU_FILE_LIST} ${GPU_ROOT_FILE}
    rm ${GPU_FILE_LIST}
    echo "Done Gpu File"
else
    rm ${GPU_FILE_LIST}
    echo "No Gpu Files"
fi
