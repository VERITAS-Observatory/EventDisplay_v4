#!/bin/bash
#
# script to check that files where processed correctly (table analysis)
#
# Revision $Id$
#
# Author: Gernot Maier
#
#######################################################################

if [ ! -n "$1" ]
then
   echo "./CTA.MSCW.check_convert_and_analyse_MC_VDST.sh <sub array>"
   echo
   echo "simple script to check that table analysis was running properly"
   echo 
   echo "  <sub array>               sub array from prod1 (e.g. E)"
   echo "                            use ALL for all arrays (A B C D E F G H I J K NA NB)"
   exit
fi

SUBAR=$1

#arrays
if [ $SUBAR = "ALL" ]
then
   VARRAY=( A B C D E F G H I J K NA NB "s4-2-120" "s4-2-85" "I-noLST" "I-noSST" "g60" "g85" "g120" "g170" "g240" "s9-2-120" "s9-2-170" )
#   VARARY=("s2-1-75" "s3-1-210" "s3-3-260" "s3-3-346" "s3-4-240" "s4-1-105" "s4-2-170" "s4-3-200" "s4-4-140" "s4-4-150" "s4-5-125" )
else
  VARRAY=( $SUBAR )
fi
NARRAY=${#VARRAY[@]}

#particles
PART=( "gamma_onSource" "gamma_cone10" "electron" "proton" )
NPART=${#PART[@]}

############################################################################

for (( N = 0 ; N < $NARRAY; N++ ))
do
   ARRAY=${VARRAY[$N]}
   echo "----------------------------------------------------"
   echo "ARRAY $ARRAY"

   for (( P = 0; P < $NPART; P++ ))
   do
      PRIM=${PART[$P]}


      DDIR="$CTA_USER_DATA_DIR/analysis/$ARRAY/Analysis/"

 # check number of root files
      NFIL=`ls $DDIR/$PRIM* | wc -l`
      echo "$NFIL $PRIM: number of files"
      NDISK=`du -h -c -s $DDIR/$PRIM* | tail -n 1`
      echo "$NDISK $PRIM: disk usage"

# check for zero length files
#      if [ ! -s $DDIR/$RUN.root ]
#      then
#        echo "ZERO LENGTH EVNDISP FILE: $DDIR/$RUN.root"
#        echo $AFIL >> $FAILED.$ARRAY.list
#        continue
#      fi
  done
   
done

exit
