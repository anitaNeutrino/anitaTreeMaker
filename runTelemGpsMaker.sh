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




echo "Starting GPS File"
ADU5_PAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/pat/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${ADU5_PAT_FILE_LIST}
    fi
done

ADU5_SAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/sat/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${ADU5_SAT_FILE_LIST}
    fi
done

cat ${ADU5_SAT_FILE_LIST}

ADU5_VTG_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/vtg/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${ADU5_VTG_FILE_LIST}
    fi
done


G12_POS_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/g12/pos/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${G12_POS_FILE_LIST}
    fi
done

G12_SAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/g12/sat/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${G12_SAT_FILE_LIST}
    fi
done

GPS_GGA_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/*/gga/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${GPS_GGA_FILE_LIST}
    fi
done

GPS_ROOT_FILE=${ROOT_RUN_DIR}/gpsFile${RUN}.root
makeGpsTree ${ADU5_PAT_FILE_LIST} ${ADU5_SAT_FILE_LIST} ${ADU5_VTG_FILE_LIST} ${G12_POS_FILE_LIST} ${G12_SAT_FILE_LIST} ${GPS_GGA_FILE_LIST} ${GPS_ROOT_FILE}
rm ${ADU5_PAT_FILE_LIST} ${ADU5_SAT_FILE_LIST} ${ADU5_VTG_FILE_LIST} ${G12_POS_FILE_LIST} ${G12_SAT_FILE_LIST} ${GPS_GGA_FILE_LIST}

ROOT_BASE_DIR=`dirname ${ROOT_RUN_DIR}`
makeGpsEventTree $ROOT_BASE_DIR ${RUN}

echo "Done GPS File"



