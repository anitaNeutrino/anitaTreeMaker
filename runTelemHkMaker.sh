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



echo "Starting Hk File"
HKCAL_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/cal/*/*/hk*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${HKCAL_FILE_LIST}
    fi
done

HKRAW_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/raw/*/*/hk*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${HKRAW_FILE_LIST}
    fi
done

HK_ROOT_FILE=${ROOT_RUN_DIR}/hkFile${RUN}.root
if  test `cat ${HKRAW_FILE_LIST} | wc -l` -gt 0 ; then
    makeHkTree ${HKCAL_FILE_LIST} ${HKRAW_FILE_LIST} ${HK_ROOT_FILE}
    rm ${HKRAW_FILE_LIST} ${HKCAL_FILE_LIST}

    if [ "$HEAD_ROOT_FILE" ]; then
	echo "Making pretty hk file (interpolated)"
	PRETTYHK_ROOT_FILE=${ROOT_RUN_DIR}/prettyHkFile${RUN}.root
	makePrettyHkTree  ${HK_ROOT_FILE} ${HEAD_ROOT_FILE} ${PRETTYHK_ROOT_FILE}
    fi
else
    rm ${HKRAW_FILE_LIST} ${HKCAL_FILE_LIST}
    echo "No Hk Files"
fi
echo "Done Hk File"


echo "Starting SSHk File"
HKCAL_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/cal/*/*/sshk*; 
do
    if [[ -f $file ]]; then
	echo $file >> ${HKCAL_FILE_LIST}
    fi
done

HKRAW_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/raw/*/*/sshk*; 
do
    if [[ -f $file ]]; then
	echo $file >> ${HKRAW_FILE_LIST}
    fi
done

HK_ROOT_FILE=${ROOT_RUN_DIR}/sshkFile${RUN}.root
if  test `cat ${HKRAW_FILE_LIST} | wc -l` -gt 0 ; then
    makeSSHkTree ${HKCAL_FILE_LIST} ${HKRAW_FILE_LIST} ${HK_ROOT_FILE}
    rm ${HKRAW_FILE_LIST} ${HKCAL_FILE_LIST}

   
else
    rm ${HKRAW_FILE_LIST} ${HKCAL_FILE_LIST}
    echo "No Hk Files"
fi
echo "Done Hk File"
