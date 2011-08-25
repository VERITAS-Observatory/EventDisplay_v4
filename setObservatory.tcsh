#!/bin/tcsh
#
# set the environmental variables to CTA/VERITAS/etc
#
# Autor: Gernot Maier
#

if ($#argv != 1) then
   echo 
   echo "set the environmental variables for EVNDISP"
   echo 
   echo "   source ./setObservatory.sh <observatory = CTA or VERITAS>"
   echo 
   exit
endif

set OBSERVATORY=$1

if( $OBSERVATORY == "VERITAS" || $OBSERVATORY == "VTS" ) then
  echo "setting observatory to VERITAS"

  setenv OBS_EVNDISP_ANA_DIR $VERITAS_EVNDISP_ANA_DIR
  setenv OBS_DATA_DIR $VERITAS_DATA_DIR
  setenv OBS_LOG_DIR $VERITAS_LOG_DIR
  setenv OBS_USER_DATA_DIR $VERITAS_USER_DATA_DIR
  setenv OBS_USER_LOG_DIR $VERITAS_USER_LOG_DIR
endif 

if( $OBSERVATORY == "CTA" ) then
  echo "setting observatory to CTA"

  setenv OBS_EVNDISP_ANA_DIR $CTA_EVNDISP_ANA_DIR
  setenv OBS_DATA_DIR $CTA_DATA_DIR
  setenv OBS_LOG_DIR $CTA_LOG_DIR
  setenv OBS_USER_DATA_DIR $CTA_USER_DATA_DIR
  setenv OBS_USER_LOG_DIR $CTA_USER_LOG_DIR
endif
