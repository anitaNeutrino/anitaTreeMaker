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


echo "Starting RTL Spectrum File"
RTL_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/rtl/*/*/rtl*
  do
  if [[ -f $file ]]; then
      echo $file >> ${RTL_FILE_LIST}
  fi
done

if  test `cat ${RTL_FILE_LIST} | wc -l` -gt 0 ; then
    ROOT_FILE=${ROOT_RUN_DIR}/rtlSpectrumFile${RUN}.root
    echo $RTL_ROOT_FILE
    makeRtlSdrTree ${RTL_FILE_LIST} ${RTL_ROOT_FILE}
    rm ${RTL_FILE_LIST}
    echo "Done RTL File"
else
    rm ${RTL_FILE_LIST}
    echo "No RTL Files"
fi
