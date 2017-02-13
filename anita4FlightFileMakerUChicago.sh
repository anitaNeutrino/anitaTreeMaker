#/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi

RUN=$1
BASE_DIR=/project/kicp/avieregg/anitaIV/flight1617
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

if [ -d $ROOT_RUN_DIR ]; then
    echo "Output dir exists"
elif [ -d $RAW_RUN_DIR  ]; then
    mkdir ${ROOT_RUN_DIR}
else
    echo "$RAW_RUN_DIR doesn't exist what are we suppposed to rootify?"
    exit 0;
fi

echo "Using $RAW_RUN_DIR"
ls ${RAW_RUN_DIR}

cd ${ANITA_UTIL_INSTALL_DIR}/bin


echo "Starting Header File"
HEAD_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/event/*/ev?/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev?????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????????/hd*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${HEAD_FILE_LIST}
  fi
done

cat ${HEAD_FILE_LIST}

if  test `cat ${HEAD_FILE_LIST} | wc -l` -gt 0 ; then
    HEAD_ROOT_FILE=${ROOT_RUN_DIR}/headFile${RUN}.root
    ./makeRawHeadTree ${HEAD_FILE_LIST} ${HEAD_ROOT_FILE}
    rm ${HEAD_FILE_LIST}
    DONE_HEAD_FILE=true
    echo "Done Header File"
else
    rm ${HEAD_FILE_LIST}
    DONE_HEAD_FILE=false
    echo "No header files"
fi

echo "Starting Event File"

EVENT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/event/*/ev?/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev?????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev??????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev???????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done
for file in ${RAW_RUN_DIR}/event/*/ev????????/psev*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${EVENT_FILE_LIST}
  fi
done

if  test `cat ${EVENT_FILE_LIST} | wc -l` -gt 0 ; then
    ./makeEventRunTree ${EVENT_FILE_LIST} ${EVENT_BASE_DIR}
    rm ${EVENT_FILE_LIST}
    echo "Done Event File"
else
    rm ${EVENT_FILE_LIST}
    echo "No event files"
fi




echo "Starting TURF Rate File"
TURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/turfhk/*/*/t*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${TURF_FILE_LIST}
  fi
done

if  test `cat ${TURF_FILE_LIST} | wc -l` -gt 0 ; then
    TURF_ROOT_FILE=${ROOT_RUN_DIR}/turfRateFile${RUN}.root
    ./makeTurfRateTree ${TURF_FILE_LIST} ${TURF_ROOT_FILE}
    rm ${TURF_FILE_LIST}
    echo "Done TURF Rate File"
else
    rm ${TURF_FILE_LIST}
    echo "No TURF Rate Files"
fi

echo "Starting SURF Hk File"
SURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/s*gz; 
  do
  if [ -f $file ]; then
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


echo "Starting Averaged SURF Hk File"
AVGSURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/a*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${AVGSURF_FILE_LIST}
  fi
done

#cat ${AVGSURF_FILE_LIST}

if  test `cat ${AVGSURF_FILE_LIST} | wc -l` -gt 0 ; then
    AVGSURF_ROOT_FILE=${ROOT_RUN_DIR}/avgSurfHkFile${RUN}.root
    ./makeAveragedSurfHkTree ${AVGSURF_FILE_LIST} ${AVGSURF_ROOT_FILE}
    rm ${AVGSURF_FILE_LIST}
    echo "Done Averaged SURF Hk File"
else
    rm ${AVGSURF_FILE_LIST}
    echo "No Averaged SURF Hk Files"
fi


echo "Starting Summed TURF Rate File"
SUMTURF_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/turfhk/*/*/s*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${SUMTURF_FILE_LIST}
  fi
done

if  test `cat ${SUMTURF_FILE_LIST} | wc -l` -gt 0 ; then
    SUMTURF_ROOT_FILE=${ROOT_RUN_DIR}/sumTurfRateFile${RUN}.root
    ./makeSummedTurfRateTree ${SUMTURF_FILE_LIST} ${SUMTURF_ROOT_FILE}
    rm ${SUMTURF_FILE_LIST}
    echo "Done SUMTURF Rate File"
else
    rm ${SUMTURF_FILE_LIST}
    echo "No Summed TURF Rate Files"
fi



echo "Starting SURF Raw Scaler File"
SCALER_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/surfhk/*/*/raw*gz; 
  do
  if [ -f $file ]; then
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


echo "Starting Monitor File"
MONITOR_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/monitor/*/*/mon*gz; 
  do
  if [ -f $file ]; then
      echo $file >> ${MONITOR_FILE_LIST}
  fi
done

if  test `cat ${MONITOR_FILE_LIST} | wc -l` -gt 0 ; then
    MONITOR_ROOT_FILE=${ROOT_RUN_DIR}/monitorFile${RUN}.root
    ./makeMonitorTree ${MONITOR_FILE_LIST} ${MONITOR_ROOT_FILE}
    rm ${MONITOR_FILE_LIST}
    echo "Done Monitor Rate File"
else
    rm ${MONITOR_FILE_LIST}
    echo "No Monitor Rate Files"
fi


echo "Starting Gpu File"
GPU_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gpu/*/*/gpu*.da*; 
  do
  if [ -f $file ]; then
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

echo "Starting Other File"
OTHER_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/monitor/*/*/other*.dat; 
  do
  if [ -f $file ]; then
      echo $file >> ${OTHER_FILE_LIST}
  fi
done

if  test `cat ${OTHER_FILE_LIST} | wc -l` -gt 0 ; then
    OTHER_ROOT_FILE=${ROOT_RUN_DIR}/monitorFile${RUN}.root
    ./makeOtherTree ${OTHER_FILE_LIST} ${OTHER_ROOT_FILE}
    rm ${OTHER_FILE_LIST}
    echo "Done Other Rate File"
else
    rm ${OTHER_FILE_LIST}
    echo "No Other Rate Files"
fi


echo "Starting Hk File"
HKCAL_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/cal/*/*/hk*; 
do
    if [ -f $file ]; then
	echo $file >> ${HKCAL_FILE_LIST}
    fi
done

HKRAW_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/raw/*/*/hk*; 
do
    if [ -f $file ]; then
	echo $file >> ${HKRAW_FILE_LIST}
    fi
done

HK_ROOT_FILE=${ROOT_RUN_DIR}/hkFile${RUN}.root
if  test `cat ${HKRAW_FILE_LIST} | wc -l` -gt 0 ; then
    ./makeHkTree ${HKCAL_FILE_LIST} ${HKRAW_FILE_LIST} ${HK_ROOT_FILE}
    rm ${HKRAW_FILE_LIST} ${HKCAL_FILE_LIST}

    if [ "$HEAD_ROOT_FILE" ]; then
	echo "Making pretty hk file (interpolated)"
	PRETTYHK_ROOT_FILE=${ROOT_RUN_DIR}/prettyHkFile${RUN}.root
	./makePrettyHkTree  ${HK_ROOT_FILE} ${HEAD_ROOT_FILE} ${PRETTYHK_ROOT_FILE}
    fi
else
    rm ${HKRAW_FILE_LIST} ${HKCAL_FILE_LIST}
    echo "No Hk Files"
fi
echo "Done Hk File"


echo "Starting SSHk File"
SSHKCAL_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/cal/*/*/sshk*; 
do
    if [ -f $file ]; then
	echo $file >> ${SSHKCAL_FILE_LIST}
    fi
done

SSHKRAW_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/hk/raw/*/*/sshk*; 
do
    if [ -f $file ]; then
	echo $file >> ${SSHKRAW_FILE_LIST}
    fi
done

SSHK_ROOT_FILE=${ROOT_RUN_DIR}/sshkFile${RUN}.root
if  test `cat ${SSHKRAW_FILE_LIST} | wc -l` -gt 0 ; then
    ./makeSSHkTree ${SSHKCAL_FILE_LIST} ${SSHKRAW_FILE_LIST} ${SSHK_ROOT_FILE}
    rm ${SSHKRAW_FILE_LIST} ${SSHKCAL_FILE_LIST}

   
else
    rm ${SSHKRAW_FILE_LIST} ${SSHKCAL_FILE_LIST}
    echo "No Hk Files"
fi
echo "Done Hk File"



echo "Starting GPS File"
ADU5_PAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/pat/*/*/*gz; 
do
    if [ -f $file ]; then
	echo $file >> ${ADU5_PAT_FILE_LIST}
    fi
done

ADU5_SAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/sat/*/*/*gz; 
do
    if [ -f $file ]; then
	echo $file >> ${ADU5_SAT_FILE_LIST}
    fi
done

cat ${ADU5_SAT_FILE_LIST}

ADU5_VTG_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/adu5?/vtg/*/*/*gz; 
do
    if [ -f $file ]; then
	echo $file >> ${ADU5_VTG_FILE_LIST}
    fi
done


G12_POS_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/g12/pos/*/*/*gz; 
do
    if [ -f $file ]; then
	echo $file >> ${G12_POS_FILE_LIST}
    fi
done

G12_SAT_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/g12/sat/*/*/*gz; 
do
    if [ -f $file ]; then
	echo $file >> ${G12_SAT_FILE_LIST}
    fi
done

GPS_GGA_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/gps/*/gga/*/*/*gz; 
do
    if [ -f $file ]; then
	echo $file >> ${GPS_GGA_FILE_LIST}
    fi
done

GPS_ROOT_FILE=${ROOT_RUN_DIR}/gpsFile${RUN}.root
./makeGpsTree ${ADU5_PAT_FILE_LIST} ${ADU5_SAT_FILE_LIST} ${ADU5_VTG_FILE_LIST} ${G12_POS_FILE_LIST} ${G12_SAT_FILE_LIST} ${GPS_GGA_FILE_LIST} ${GPS_ROOT_FILE}
rm ${ADU5_PAT_FILE_LIST} ${ADU5_SAT_FILE_LIST} ${ADU5_VTG_FILE_LIST} ${G12_POS_FILE_LIST} ${G12_SAT_FILE_LIST} ${GPS_GGA_FILE_LIST}
echo "Done GPS File"



echo "Starting Auxiliary File"
ACQD_START_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/start/a*; 
do
    if [ -f $file ]; then
	echo $file >> ${ACQD_START_FILE_LIST}
    fi
done

GPSD_START_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/start/g*; 
do
    if [ -f $file ]; then
	echo $file >> ${GPSD_START_FILE_LIST}
    fi
done

LOGWATCHD_START_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/start/l*; 
do
    if [ -f $file ]; then
	echo $file >> ${LOGWATCHD_START_FILE_LIST}
    fi
done


CMD_ECHO_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/cmd/*/*/*gz; 
do
    if [ -f $file ]; then
	echo $file >> ${CMD_ECHO_FILE_LIST}
    fi
done

AUX_ROOT_FILE=${ROOT_RUN_DIR}/auxFile${RUN}.root
./makeAuxiliaryTree ${ACQD_START_FILE_LIST} ${GPSD_START_FILE_LIST} ${LOGWATCHD_START_FILE_LIST} ${CMD_ECHO_FILE_LIST} ${AUX_ROOT_FILE}
rm ${ACQD_START_FILE_LIST} ${GPSD_START_FILE_LIST} ${LOGWATCHD_START_FILE_LIST} ${CMD_ECHO_FILE_LIST}
echo "Done Auxiliary File"

echo "Start Tuff Status File" 
TUFF_STATUS_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/tuff/*/*/tuff*; 
do 
  if [ -f $file ]; then 
    echo $file >> ${TUFF_STATUS_FILE_LIST}
  fi
done

TUFF_STATUS_ROOT_FILE=${ROOT_RUN_DIR}/tuffStatusFile${RUN}.root
./makeTuffStatusTree ${TUFF_STATUS_FILE_LIST} ${TUFF_STATUS_ROOT_FILE}
rm ${TUFF_STATUS_FILE_LIST} 
echo "Done Tuff Status File" 

echo "Start RTL Spectrum File" 
RTL_FILE_LIST=`mktemp`
for file in ${RAW_RUN_DIR}/house/rtl/*/*/rtl*; 
do 
  if [ -f $file ]; then 
    echo $file >> ${RTL_FILE_LIST}
  fi
done

RTL_ROOT_FILE=${ROOT_RUN_DIR}/rtlSpectrumFile${RUN}.root
./makeRtlSdrTree ${RTL_FILE_LIST} ${RTL_ROOT_FILE}
rm ${RTL_FILE_LIST} 
echo "Done RTL Spectrum File" 



