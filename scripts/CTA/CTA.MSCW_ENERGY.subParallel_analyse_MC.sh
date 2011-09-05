#!/bin/bash
#
# submit jobs to analyse MC files with lookup tables
# divide large set of e.g. proton simulations into smaller sets
#
#

TAB=$1
RECID=$2
ARRAY=$3
PART=$4
MET=$5

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ] && [ ! -n "$5" ]
then
   echo "./CTA.MSCW_ENERGY.subParallel_analyse_MC.sh <tablefile> <recid> <subarray> <particle> <LL/GEO>"
   echo "(table files without .root)"
   echo ""
   echo "submit jobs in paralell to analyse MC files with lookup tables"
   echo
   echo "  <tablefile>     table file name (without .root)"
   echo "  <recid>         reconstruction ID"
   echo "  <subarray>      subarray identifier (A,B,C...)"
   echo "  <particle>      gamma_onSource / gamma_cone10 / electron / proton / helium"
   echo "  <LL/GEO>        image reconstruction method (not implemented yet)"
   echo
   exit
fi

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
NRR=${#RR[@]}

# loop over all data sets and submit jobs
for ((j = 0; j < $NRR; j++ ))
do
   I=${RR[$j]}
   for (( k = 0; k < 10; k++ ))
   do
      ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TAB $RECID $ARRAY $PART $MET $I$k "$I$k"
   done
done

exit
