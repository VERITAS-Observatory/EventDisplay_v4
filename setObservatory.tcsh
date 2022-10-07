#!/bin/tcsh
#
# set the environmental variables to CTA/VERITAS/etc
#

if ($#argv != 1) then
   echo 
   echo "set the environmental variables for EVNDISP"
   echo 
   echo "   source ./setObservatory.sh <observatory = CTA or VERITAS/VTS>"
   echo 
   exit
endif

set OBSERVATORY=$1

######################################################
## dependencies
######################################################

# root LD_LIBRARY_PATH
if ($?LD_LIBRARY_PATH) then
    setenv LD_LIBRARY_PATH $ROOTSYS/lib/:$LD_LIBRARY_PATH
else
    setenv LD_LIBRARY_PATH $ROOTSYS/lib/
endif

# GSL LIBRARIES
setenv LD_LIBRARY_PATH /opt/products/gsl/1.9/lib64/:$LD_LIBRARY_PATH

# VBF
if ( $?VBFSYS ) then
   setenv LD_LIBRARY_PATH $VBFSYS/lib:$LD_LIBRARY_PATH
endif

# HESSIOSYS needed for reading of CTA sim_telarray files
if( $OBSERVATORY == "CTA" ) then
  setenv LD_LIBRARY_PATH $HESSIOSYS/lib:$LD_LIBRARY_PATH
endif

# SOFASYS
setenv SOFASYS $EVNDISPSYS/sofa

######################################################
## EVNDISP libraries 
setenv LD_LIBRARY_PATH $EVNDISPSYS/lib/:$EVNDISPSYS/obj/:$LD_LIBRARY_PATH
######################################################
