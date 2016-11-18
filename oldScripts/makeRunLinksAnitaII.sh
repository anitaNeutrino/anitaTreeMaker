#/bin/bash

RAW_BASE_DIR=/unix/anita3/flight0809/raw
DRIVE_BASE_NAME=satablade

for RUN_DIR in ${RAW_BASE_DIR}/${DRIVE_BASE_NAME}?/run*;
  do
#  echo $RUN_DIR
  JUST_RUN=`basename $RUN_DIR`
  LINK_DIR=${RAW_BASE_DIR}/${JUST_RUN}
#  echo $LINK_DIR
  ln -sf $RUN_DIR $LINK_DIR
done
