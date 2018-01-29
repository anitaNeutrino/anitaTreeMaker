#/bin/bash
# to run this from command line type ./runMakeCalibratedEventInfo.sh start_run end_run
# where start_run is the first run you want to make calibratedEventInfo for, and end_run is the last

for i in `seq $1 $2`
do
  build/makeCalibratedEventInfo $ANITA_ROOT_DATA $ANITA_ROOT_DATA $i
done
