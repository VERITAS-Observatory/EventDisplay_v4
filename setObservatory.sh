#!/bin/sh
#
# set the environmental variables to CTA/VERITAS/etc
#
# Autor: Gernot Maier
#

if [ ! -n "$1" ]
then
   echo 
   echo "set the environmental variables for EVNDISP"
   echo 
   echo "   source ./setObservatory.sh <observatory = CTA or VERITAS>"
   echo 
   exit
fi
OBSERVATORY=$1

if [ $OBSERVATORY = "VERITAS" ] || [ $OBSERVATORY = "VTS" ]
then
  echo "setting observatory to VERITAS"

  export OBS_EVNDISP_ANA_DIR=$VERITAS_EVNDISP_ANA_DIR
  export OBS_DATA_DIR=$VERITAS_DATA_DIR
  export OBS_LOG_DIR=$VERITAS_LOG_DIR
  export OBS_USER_DATA_DIR=$VERITAS_USER_DATA_DIR
  export OBS_USER_LOG_DIR=$VERITAS_USER_LOG_DIR
fi

if [ $OBSERVATORY = "CTA" ]
then
  echo "setting observatory to CTA"

  export OBS_EVNDISP_ANA_DIR=$CTA_EVNDISP_ANA_DIR
  export OBS_DATA_DIR=$CTA_DATA_DIR
  export OBS_LOG_DIR=$CTA_LOG_DIR
  export OBS_USER_DATA_DIR=$CTA_USER_DATA_DIR
  export OBS_USER_LOG_DIR=$CTA_USER_LOG_DIR
fi

