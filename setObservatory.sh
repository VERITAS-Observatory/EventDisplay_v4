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

# root LD_LIBRARY_PATH
if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$ROOTSYS/lib; export LD_LIBRARY_PATH 
else
   LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

# EVNDISP libraries 
LD_LIBRARY_PATH=$EVNDISPSYS/lib/:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
# GSL LIBRARIES
LD_LIBRARY_PATH=/opt/products/gsl/1.9/lib64/:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH 
# VBF
LD_LIBRARY_PATH=$VBFSYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH 

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
# HESSIOSYS
   LD_LIBRARY_PATH=$HESSIOSYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH 
fi

