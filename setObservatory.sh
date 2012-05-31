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
   echo "   source ./setObservatory.sh <observatory = CTA or VERITAS/VTS>"
   echo 
   exit
fi
OBSERVATORY=$1

######################################################
## dependencies
######################################################

# root LD_LIBRARY_PATH
if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$ROOTSYS/lib; export LD_LIBRARY_PATH 
else
   LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

# GSL LIBRARIES
LD_LIBRARY_PATH=/opt/products/gsl/1.9/lib64/:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH 

# VBF
if [ -z "$VBFSYS" ]; then
   LD_LIBRARY_PATH=$VBFSYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH 
fi

# HESSIOSYS needed for reading of CTA sim_telarray files
if [ $OBSERVATORY = "CTA" ]
then
   LD_LIBRARY_PATH=$HESSIOSYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH 
fi

######################################################
## EVNDISP libraries 
LD_LIBRARY_PATH=$EVNDISPSYS/lib/:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
######################################################

######################################################
## environmental variables
######################################################
if [ $OBSERVATORY = "VERITAS" ] || [ $OBSERVATORY = "VTS" ]
then
  echo "setting observatory to VERITAS"

  if [ -z "${VERITAS_EVNDISP_AUX_DIR+xxx}" ]; then 
     export OBS_EVNDISP_ANA_DIR=$VERITAS_EVNDISP_ANA_DIR
  else
     export OBS_EVNDISP_ANA_DIR=$VERITAS_EVNDISP_AUX_DIR
  fi
  export OBS_DATA_DIR=$VERITAS_DATA_DIR
  export OBS_USER_DATA_DIR=$VERITAS_USER_DATA_DIR
  export OBS_USER_LOG_DIR=$VERITAS_USER_LOG_DIR
fi

if [ $OBSERVATORY = "CTA" ]
then
  echo "setting observatory to CTA"
  if [ -z "${CTA_EVNDISP_AUX_DIR+xxx}" ]; then 
     export OBS_EVNDISP_ANA_DIR=$CTA_EVNDISP_ANA_DIR
  else
     export OBS_EVNDISP_ANA_DIR=$CTA_EVNDISP_AUX_DIR
  fi
  export OBS_EVNDISP_ANA_DIR=$CTA_EVNDISP_ANA_DIR
  export OBS_DATA_DIR=$CTA_DATA_DIR
  export OBS_USER_DATA_DIR=$CTA_USER_DATA_DIR
  export OBS_USER_LOG_DIR=$CTA_USER_LOG_DIR
fi

