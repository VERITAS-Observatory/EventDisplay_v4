#!/bin/bash
#
# submit jobs for effective area calculation
#
#


if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$4" ] && [ ! -n "$4" ]
then
   echo ""
   echo "./CTA.EFFAREA.subAllParticle_analyse.sh <subarray list> <cutfile template> <output directory> <data set> [filling mode]"
   echo
   echo "<subarray list>"
   echo "     text file with list of subarray IDs"
   echo
   echo "<cutfile template>"
   echo "     template for gamma/hadron cut file"
   echo "     (suffix must be .gamma/.CRbck ; this will be added by this script)"
   echo 
   echo "<output directory>"
   echo "     directory name for output effective areas files"
   echo
   echo " <data set>         e.g. cta-ultra3, ISDC3700m, ...  "
   echo
   echo "[filling mode]"
   echo "effective area filling mode (use 2 to calculate angular resolution only)"
   echo
   echo ""
   exit
fi

SUBAR=$1
RECID=0
CDIR="$CTA_EVNDISP_ANA_DIR/ParameterFiles/"
CFIL=$2
ODIR=$3
GMOD=0
DSET=$4
if [ -n "$5" ]
then
  GMOD=$5
fi
mkdir -p $ODIR

#arrays
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $SUBAR`

# particle types
if [ $GMOD = "0" ]
then
   if [ $DSET = "cta-ultra3" ]
   then
      VPART=( "gamma_onSource" "gamma_cone10" "electron" "proton" )
   else
      VPART=( "gamma_onSource" "electron" "proton" )
   fi
else
   if [ $DSET = "cta-ultra3" ]
   then
      VPART=( "gamma_onSource" "gamma_cone10" )
   else
      VPART=( "gamma_onSource" )
   fi
fi
NPART=${#VPART[@]}

#########################################
#loop over all arrays
#########################################
for ARRAY in $VARRAY
do
   echo "STARTING ARRAY $ARRAY"

###########################################
# loop over all particle types
   for ((m = 0; m < $NPART; m++ ))
   do
      PART=${VPART[$m]}
      echo "MC PARTICLE $PART"

      CCUT=$ODIR/$CFIL.$PART.$ARRAY.dat
      if [ $PART = "gamma_onSource" ] || [ $PART = "gamma_cone10" ]
      then
        cp $CDIR/$CFIL.gamma.dat $CCUT
      fi
      if [ $PART = "proton" ] || [ $PART = "electron" ] || [ $PART = "helium" ] 
      then
        cp $CDIR/$CFIL.CRbck.dat $CCUT
      fi

      ./CTA.EFFAREA.sub_analyse.sh $ARRAY $RECID $PART $CCUT $ODIR $DSET $GMOD
   done
done

exit
