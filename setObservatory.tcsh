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

# root LD_LIBRARY_PATH
if ($?LD_LIBRARY_PATH) then
    setenv LD_LIBRARY_PATH $ROOTSYS/lib/:$LD_LIBRARY_PATH
else
    setenv LD_LIBRARY_PATH $ROOTSYS/lib/
endif

# EVNDISP libraries
setenv LD_LIBRARY_PATH $EVNDISPSYS/lib/:$LD_LIBRARY_PATH
# GSL LIBRARIES
setenv LD_LIBRARY_PATH /opt/products/gsl/1.9/lib64/:$LD_LIBRARY_PATH
# VBF
setenv LD_LIBRARY_PATH $VBFSYS/lib:$LD_LIBRARY_PATH

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
# HESSIOSYS
  setenv LD_LIBRARY_PATH $HESSIOSYS/lib:$LD_LIBRARY_PATH
endif

