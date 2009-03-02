#/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi

RUN=$1
BASE_DIR=/unix/anita3/flight0809
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
NEO_RUN_DIR=${BASE_DIR}/neoraw/run${RUN}


echo "Starting Header File"
HEAD_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/event/*/ev??/hd*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???/hd*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????/hd*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev?????/hd*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??????/hd*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???????/hd*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????????/hd*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done




for mtronfile in `cat $HEAD_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $HEAD_FILE_LIST

echo "Starting Event File"

EVENT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/event/*/ev??/psev*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???/psev*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????/psev*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev?????/psev*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??????/psev*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???????/psev*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????????/psev*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done

for mtronfile in `cat $EVENT_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $EVENT_FILE_LIST

echo "Starting TURF Rate File"
TURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/turfhk/*/*/t*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${TURF_FILE_LIST}
  fi
done

for mtronfile in `cat $TURF_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $TURF_FILE_LIST

echo "Starting SURF Hk File"
SURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/s*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${SURF_FILE_LIST}
  fi
done

for mtronfile in `cat $SURF_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $SURF_FILE_LIST


echo "Starting Averaged SURF Hk File"
AVGSURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/a*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${AVGSURF_FILE_LIST}
  fi
done


for mtronfile in `cat $AVGSURF_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $AVGSURF_FILE_LIST


echo "Starting Summed TURF Rate File"
SUMTURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/turfhk/*/*/s*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${SUMTURF_FILE_LIST}
  fi
done

for mtronfile in `cat $SUMTURF_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $SUMTURF_FILE_LIST



echo "Starting SURF Raw Scaler File"
SCALER_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/raw*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${SCALER_FILE_LIST}
  fi
done


for mtronfile in `cat $SCALER_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $SCALER_FILE_LIST



echo "Starting Monitor File"
MONITOR_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/monitor/*/*/mon*gz; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${MONITOR_FILE_LIST}
  fi
done



for mtronfile in `cat $MONITOR_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $MONITOR_FILE_LIST

echo "Starting Other File"
OTHER_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/monitor/*/*/other*.dat; 
  do
  if [[ -f $file ]]; then
      echo $file >> ${OTHER_FILE_LIST}
  fi
done


for mtronfile in `cat $OTHER_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $OTHER_FILE_LIST

echo "Starting Hk File"
HKCAL_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/cal/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${HKCAL_FILE_LIST}
    fi
done


for mtronfile in `cat $HKCAL_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $HKCAL_FILE_LIST


HKRAW_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/raw/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${HKRAW_FILE_LIST}
    fi
done


for mtronfile in `cat $HKRAW_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done

rm $HKRAW_FILE_LIST

echo "Starting GPS File"
ADU5_PAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/pat/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${ADU5_PAT_FILE_LIST}
    fi
done


for mtronfile in `cat $ADU5_PAT_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $ADU5_PAT_FILE_LIST

ADU5_SAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/sat/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${ADU5_SAT_FILE_LIST}
    fi
done


for mtronfile in `cat $ADU5_SAT_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $ADU5_SAT_FILE_LIST

ADU5_VTG_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/vtg/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${ADU5_VTG_FILE_LIST}
    fi
done



for mtronfile in `cat $ADU5_VTG_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $ADU5_VTG_FILE_LIST


G12_POS_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/g12/pos/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${G12_POS_FILE_LIST}
    fi
done


for mtronfile in `cat $G12_POS_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $G12_POS_FILE_LIST


G12_SAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/g12/sat/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${G12_SAT_FILE_LIST}
    fi
done


for mtronfile in `cat $G12_SAT_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done

rm $G12_SAT_FILE_LIST

GPS_GGA_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/*/gga/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${GPS_GGA_FILE_LIST}
    fi
done

for mtronfile in `cat $GPS_GGA_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $GPS_GGA_FILE_LIST

CMD_ECHO_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/cmd/*/*/*gz; 
do
    if [[ -f $file ]]; then
	echo $file >> ${CMD_ECHO_FILE_LIST}
    fi
done


for mtronfile in `cat $CMD_ECHO_FILE_LIST`;  
  do
#  echo $mtronfile;
  topPart=${mtronfile##*/flight0809/raw/}
  neofile=$BASE_DIR/neoraw/$topPart
#  echo $neofile
 
  mtronsum=`md5sum $mtronfile | awk '{ print $1 }'`
  neosum=`md5sum $neofile | awk '{ print $1 }'`

  if [ "$mtronsum" != "$neosum" ]; then
      echo $neofile
      echo $mtronsum $neosum
  fi
done
rm $CMD_ECHO_FILE_LIST
