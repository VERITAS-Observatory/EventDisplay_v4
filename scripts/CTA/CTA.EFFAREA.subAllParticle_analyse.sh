#!/bin/bash
#
# submit jobs for effective area calculation
#
#


if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ] 
then
   echo ""
   echo "./CTA.EFFAREA.subAllParticle_analyse.sh <subarray> <cut file directory> <cutfile template> <output directory> [filling mode]"
   echo
   echo "<subarray>"
   echo "     subarray identifier (A,B,C...)"
   echo "     use ALL for all arrays (A B C D E F G H I J K NA NB)"
   echo
   echo "<cut file directory>"
   echo "     direction where cut files are located"  
   echo "<cutfile template>"
   echo "     template for gamma/hadron cut file"
   echo "     (suffix must be .gamma_onSource/.gamma_cone10/.CRbck ; this will be added by this script)"
   echo 
   echo "<output directory>"
   echo "     directory name for output effective areas files"
   echo
   echo "[filling mode]"
   echo "effective area filling mode (use 2 to calculate angular resolution only"
   echo
   echo ""
   exit
fi

SUBAR=$1
RECID=0
CDIR=$2
CFIL=$3
ODIR=$4
GMOD=0
if [ -n "$5" ]
then
  GMOD=$5
fi
mkdir -p $ODIR

#arrays
if [ $SUBAR = "ALL" ]
then
#  VARRAY=( A B C D E F G H I J K NA NB "s4-2-120" "s4-2-85" "I-noLST" "I-noSST" "g60" "g85" "g120" "g170" "g240" "s9-2-120" "s9-2-170" )
  VARRAY=( E I J )
#  VARRAY=( "s2-1-75" "s3-1-210" "s3-3-260" "s3-3-346" "s3-4-240" "s4-1-105" "s4-2-170" "s4-3-200" "s4-4-140" "s4-4-150" "s4-5-125" )
else
  VARRAY=( $SUBAR )
fi
NARRAY=${#VARRAY[@]}

# particle types
#if [ $GFILL = "0" ]
if [ $GMOD = "0" ]
then
#   VPART=( "gamma_onSource" "gamma_cone10" "electron" "proton" "helium" )
   VPART=( "gamma_onSource" "gamma_cone10" "electron" "proton" )
else
   VPART=( "gamma_onSource" "gamma_cone10" )
fi
NPART=${#VPART[@]}

#########################################
#loop over all arrays
#########################################
for (( N = 0 ; N < $NARRAY; N++ ))
do
   ARRAY=${VARRAY[$N]}
   echo "STARTING ARRAY $ARRAY"

###########################################
# loop over all particle types
   for ((m = 0; m < $NPART; m++ ))
   do
      PART=${VPART[$m]}
      echo "MC PARTICLE $PART"

      CCUT=$ODIR/$CFIL.$PART.$ARRAY.dat
      if [ $PART = "gamma_onSource" ]
      then
        cp $CDIR/$CFIL.gamma_onSource.dat $CCUT
      fi
      if [ $PART = "gamma_cone10" ]
      then
        cp $CDIR/$CFIL.gamma_cone10.dat $CCUT
      fi
      if [ $PART = "proton" ] || [ $PART = "electron" ] || [ $PART = "helium" ] 
      then
        cp $CDIR/$CFIL.CRbck.dat $CCUT
      fi

      ./CTA.EFFAREA.sub_analyse.sh $ARRAY $RECID $PART $CCUT $ODIR $GMOD
   done
done

exit
