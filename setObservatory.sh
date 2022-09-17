#!/bin/sh
#
# set the environmental variables to CTA/VERITAS/etc
#

if [ ! -n "$1" ] || [ "$1" = "-h" ] || [ "$1" = "-help" ]
then
   echo 
   echo "set the environmental variables for EVNDISP"
   echo 
   echo "   source ./setObservatory.sh <observatory = CTA or VERITAS/VTS> [-q for no output if successful]"
   echo 
   echo "    (this is needed to find e.g. the parameter or lookup table files)"
   echo 
   if [[ "${BASH_SOURCE[0]}" != "${0}" ]]
   then
      kill -INT $$
   else
      exit
   fi
fi
OBSERVATORY=$1

if [ -z "${EVNDISPSYS}" ]; then
    echo "Error: required environmental variable EVNDISPSYS not set"
    echo "exiting..."
    return
fi

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
if [ ! -z "$VBFSYS" ]; then
   LD_LIBRARY_PATH=$VBFSYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH 
fi

# HESSIOSYS needed for reading of CTA sim_telarray files
if [ $OBSERVATORY = "CTA" ]
then
   LD_LIBRARY_PATH=$HESSIOSYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH 
fi

# SOFASYS
export SOFASYS=$EVNDISPSYS/sofa

######################################################
## EVNDISP libraries 
LD_LIBRARY_PATH=$EVNDISPSYS/lib/:$EVNDISPSYS/obj/:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
######################################################
