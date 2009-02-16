#/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi

RUN=$1
RAW_BASE_DIR=/unix/anita/rawData/postFlight/run${RUN}
ROOT_BASE_DIR=/unix/anita1/postFlight/run${RUN}

if [[ -d $ROOT_BASE_DIR ]]; then
    echo "Output dir exists"
else
    mkdir ${ROOT_BASE_DIR}
fi


echo "Starting Header File"
HEAD_FILE_LIST=`mktemp`
for file in ${RAW_BASE_DIR}/event/*/*/hd*; 
  do
  echo $file >> ${HEAD_FILE_LIST}
done

HEAD_ROOT_FILE=${ROOT_BASE_DIR}/headFile${RUN}.root
./makeRawHeadTree ${HEAD_FILE_LIST} ${HEAD_ROOT_FILE}
rm ${HEAD_FILE_LIST}
echo "Done Header File"


echo "Starting Monitor File"
MONITOR_FILE_LIST=`mktemp`
for file in ${RAW_BASE_DIR}/house/monitor/*/*/mon*; 
  do
  echo $file >> ${MONITOR_FILE_LIST}
done

MONITOR_ROOT_FILE=${ROOT_BASE_DIR}/monitorFile${RUN}.root
./makeMonitorTree ${MONITOR_FILE_LIST} ${MONITOR_ROOT_FILE}
rm ${MONITOR_FILE_LIST}

OTHER_FILE_LIST=`mktemp`
for file in ${RAW_BASE_DIR}/house/monitor/*/*/other*; 
  do
  echo $file >> ${OTHER_FILE_LIST}
done

OTHER_ROOT_FILE=${ROOT_BASE_DIR}/monitorFile${RUN}.root
./makeOtherTree ${OTHER_FILE_LIST} ${OTHER_ROOT_FILE}
rm ${OTHER_FILE_LIST}
echo "Done Monitor File"

echo "Starting Hk File"
HKCAL_FILE_LIST=`mktemp`
for file in ${RAW_BASE_DIR}/house/hk/cal/*/*/*; 
  do
  echo $file >> ${HKCAL_FILE_LIST}
done

HKRAW_FILE_LIST=`mktemp`
for file in ${RAW_BASE_DIR}/house/hk/raw/*/*/*; 
  do
  echo $file >> ${HKRAW_FILE_LIST}
done

HK_ROOT_FILE=${ROOT_BASE_DIR}/hkFile${RUN}.root
PRETTYHK_ROOT_FILE=${ROOT_BASE_DIR}/prettyHkFile${RUN}.root
./makeHkTree ${HKCAL_FILE_LIST} ${HKRAW_FILE_LIST} ${HK_ROOT_FILE}
./makePrettyHkTree  ${HK_ROOT_FILE} ${PRETTYHK_ROOT_FILE}
echo "Done Hk File"


echo "Starting TURF Rate File"
TURF_FILE_LIST=`mktemp`
for file in ${RAW_BASE_DIR}/house/turfhk/*/*/*; 
  do
  echo $file >> ${TURF_FILE_LIST}
done

TURF_ROOT_FILE=${ROOT_BASE_DIR}/turfRateFile${RUN}.root
./makeTurfRateTree ${TURF_FILE_LIST} ${TURF_ROOT_FILE}
rm ${TURF_FILE_LIST}
echo "Done TURF Rate File"

echo "Starting SURF Hk File"
SURF_FILE_LIST=`mktemp`
for file in ${RAW_BASE_DIR}/house/surfhk/*/*/*; 
  do
  echo $file >> ${SURF_FILE_LIST}
done

SURF_ROOT_FILE=${ROOT_BASE_DIR}/surfHkFile${RUN}.root
./makeSurfHkTree ${SURF_FILE_LIST} ${SURF_ROOT_FILE}
rm ${SURF_FILE_LIST}
echo "Done SURF Hk File"
