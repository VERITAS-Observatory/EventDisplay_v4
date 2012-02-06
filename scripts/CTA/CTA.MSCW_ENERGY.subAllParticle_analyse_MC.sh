#!/bin/bash
#
# submit jobs to analyse MC files with lookup tables for all particle types
# divide large set of e.g. proton simulations into smaller sets
#
#

TAB=$1
RECID=$2
ARRAY=$3
PART=$4
MET=$5

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ]
then
   echo ""
   echo "./CTA.MSCW_ENERGY.subAllParticle_analyse_MC.sh <tablefile> <recid> <subarray list> <data set>"
   echo "(table files without .root)"
   echo ""
   echo "submit jobs in paralell to analyse MC files with lookup tables"
   echo
   echo "  <tablefile>     table file name (without .root)"
   echo "  <recid>         reconstruction ID"
   echo "  <data set>      e.g. ultra, ISDC3700m, ..."
   echo ""
   exit
fi

#VPART=( "gamma_onSource" "gamma_cone10" "electron" "proton" "helium" )
VPART=( "proton" )
VPART=( "gamma_onSource" "gamma_cone10" "electron" "proton" )
NPART=${#VPART[@]}

###########################################
# loop over all particle types
for ((m = 0; m < $NPART; m++ ))
do
   PART=${VPART[$m]}

   for (( k = 0; k < 10; k++ ))
   do
      if [ $4 = "v_leeds" ]
      then
	 echo "v_leeds 1$k"
	 for (( l = 0; l < 10; l++ ))
	 do
	    ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TAB $RECID $ARRAY $PART $4 $k$l
         done
      else
	 ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TAB $RECID $ARRAY $PART $4 $k
      fi
   done
done

exit
