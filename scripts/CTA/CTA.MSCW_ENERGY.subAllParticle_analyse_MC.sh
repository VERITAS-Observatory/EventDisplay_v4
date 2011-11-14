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

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]
then
   echo ""
   echo "./CTA.MSCW_ENERGY.subAllParticle_analyse_MC.sh <tablefile> <recid> <subarray>"
   echo "(table files without .root)"
   echo ""
   echo "submit jobs in paralell to analyse MC files with lookup tables"
   echo
   echo "  <tablefile>     table file name (without .root)"
   echo "  <recid>         reconstruction ID"
   echo "  <subarray>      subarray identifier (A,B,C...)"
   echo "                  use ALL for all arrays (A B C D E F G H I J K NA NB)"
   echo ""
   exit
fi

VPART=( "gamma_onSource" "gamma_cone10" "electron" "proton" "helium" )
NPART=${#VPART[@]}

###########################################
# loop over all particle types
for ((m = 0; m < $NPART; m++ ))
do
   PART=${VPART[$m]}

# proton data set
   if [ $PART = "proton" ]
   then
     RR=( 3 4 5 6 7 8 9 10 11 12 )
   fi
# gamma rays (isotropic simulations)
   if [ $PART = "gamma_cone10" ]
   then
      RR=( 12 13 )
   fi
# gamma rays (on source)
   if [ $PART = "gamma_onSource" ]
   then
      RR=( 3 4 5 )
   fi
# one single output file
   if [ $PART = "electron" ]
   then
      RR=( 3 4 5 )
   fi
   if [ $PART = "helium" ]
   then
      RR=( 5 )
   fi
   NRR=${#RR[@]}

   echo $PART $NRR

###########################################
# loop over all data sets and submit jobs
   if [ $NRR -gt 0 ]
   then
      for ((j = 0; j < $NRR; j++ ))
      do
	 I=${RR[$j]}
	 for (( k = 0; k < 10; k++ ))
	 do
	    ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TAB $RECID $ARRAY $PART $I$k
	 done
      done
   else
      ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TAB $RECID $ARRAY $PART
   fi
done

exit
