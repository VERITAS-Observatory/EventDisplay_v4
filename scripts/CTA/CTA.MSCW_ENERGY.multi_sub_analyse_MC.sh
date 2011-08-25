#!/bin/bash
#
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ] && [ ! -n "$5" ]
then
   echo "CTA.MSCW_ENERGY.multi_sub_analyse_MC_VDST.sh <gamma_onSource/gamma_cone10/electron/proton/helium> <table filename> <subarray> <recID> <GEO/LL>"
   echo 
   echo "(table files without .root)"
   exit
fi

PART=$1
TAB=$2
ARRAY=$3
RECID=$4
MET=$5

### directory structure looks like:
### $CTA_USER_DATA_DIR/analysis/E/proton/10000.root

if [ $PART = "proton" ]
then
  RR=( 3 4 5 6 7 8 9 10 11 12 )
fi

if [ $PART = "gamma_cone10" ]
then
  RR=( 12 13 )
fi

NRR=${#RR[@]}

for ((j = 0; j < $NRR; j++ ))
do
   I=${RR[$j]}
   for (( k = 0; k < 10; k++ ))
   do
      ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TAB $RECID $ARRAY $PART $MET "$I$k"
   done
done

exit
