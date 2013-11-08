#!/bin/sh
#
# script to analyse data files with anasum (parallel analysis) from a simple run list
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ] 
then
   echo
   echo "VTS.ANASUM.sub_analyseParallel_data.sh <run list> <data directory with mscw files> <run parameter file> <output directory for anasum products> <cut set/background model>"
   echo
   echo "   <run list>     simple run list "
   echo 
   echo "   <cut set/background model>"
   echo "                  cut sets predifined and hardwired in the script:"
   echo "                         soft, moderate"
   echo "                  background model: RE, RB"
   echo 
   echo "   <run parameter file>  anasum run parameter file"
   echo "                         (example can be found in $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/ANASUM.runparameter.dat)"
   echo
   echo "  runlist should contain run numbers only"
   echo
   echo "  example for run list:"
   echo "    48626"
   echo "    58453"
   echo "    61429"
   echo
   echo "  NOTE: HARDWIRED IRF FILE NAMES"
   echo
   exit
fi

RLIST=$1
DDIR=$2
RUNP=$3
ODIR=$4
CUTS=$5
ATMO="ATM21"
if [[ "$CUTS" == *22* ]]
then
  ATMO="ATM22"
fi

#########################################
# cut definitions
if [[ "$CUTS" == *super* ]]
then
   CUTFILE="ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-SuperSoft.dat"
   EFFAREA="effArea-d20131031-cut-N2-Point-005CU-SuperSoft-$ATMO-EPOCHSETTING-T1234-d20131031.root"
   RADACC="radialAcceptance-d20131031-d20131028-cut-N2-Point-005CU-SuperSoft-V6-T1234.root"
elif [[ "$CUTS" == *open* ]]
then
   CUTFILE="ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open.dat"
   EFFAREA="effArea-d20131031-cut-N2-Point-005CU-Open-$ATMO-EPOCHSETTING-T1234-d20131031.root"
   RADACC="radialAcceptance-d20130411-cut-N3-Point-005CU-Moderate-EPOCHSETTING-T1234.root"
elif [[ "$CUTS" == *soft* ]]
then
   CUTFILE="ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft.dat"
   EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Soft-$ATMO-EPOCHSETTING-T1234-d20131031.root"
   RADACC="radialAcceptance-d20131031-d20131028-cut-N3-Point-005CU-Soft-V6-T1234.root"
elif [[ $CUTS = *moderate* ]]
then
   CUTFILE="ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate.dat"
   EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Moderate-$ATMO-EPOCHSETTING-T1234-d20131031.root"
   RADACC="radialAcceptance-d20130411-cut-N3-Point-005CU-Moderate-EPOCHSETTING-T1234.root"
# UV Filter
#   EFFAREA="effArea-d20120909-cut-N3-Point-005CU-Moderate-V6-$ATMO-UV-d20121218.root"
#   RADACC="radialAcceptance-d20120909-cut-N3-Point-005CU-Moderate-EPOCHSETTING-d20121218.root"
elif [[ $CUTS = *hard* ]]
then
  CUTFILE="ANASUM.GammaHadron.d20120909-cut-N3-Point-005CU-Hard.dat"
  EFFAREA="effArea-d20120909-cut-N3-Point-005CU-Moderate-EPOCHSETTING-d20121218.root"
  RADACC="radialAcceptance-d20120909-cut-N3-Point-005CU-Moderate-EPOCHSETTING-d20121218.root"
# energy dependent theta2 cut
elif [[ $CUTS = *VT2* ]]
then
  CUTFILE="ANASUM.GammaHadron.d20130411-cut-N3-Point-VariableTheta2-Moderate.dat"
  EFFAREA="eff-N3-VT2-moderate-21-V6-0-0-20-0.5-200.root"
  RADACC="radialAcceptance-d20130411-d20130411-cut-N3-Point-005CU-Moderate-EPOCHSETTING-T1234.root"
else
   echo "error: unknown cut definition: $CUTS"
   echo "    allowed are *soft*, *moderate*"
   exit
fi
   

#########################################
# background model parameters
if [[ "$CUTS" == *RE* ]]
then
   BM="RE"
   BMPARA="0.1 3 8"
   BMPARA="0.1 1 20"
   BMPARA="0.1 2 6"
elif [[ "$CUTS" == *RB* ]]
then
   BM="RB"
   BMPARA="0.7 20"
else
   echo "error: unknown background model: $CUTS"
   echo "    allowed are: RE, RB"
   exit
fi
  
# checking the path for binaries and scripts
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

#########################################
# directory for run scripts
DATE=`date +"%y%m%d-%H%M%S"`
LDIR=$VERITAS_USER_LOG_DIR"/queueAnasum/"$DATE"/"
mkdir -p $LDIR

#########################################
# output directory for anasum products
mkdir -p $ODIR

#########################################
# make anasum run list
LLIST="$CUTS.anasum.dat"
rm -f $ODIR/$LLIST
touch $ODIR/$LLIST
echo "anasum run list: $ODIR/$LLIST"

RUNS=`cat $RLIST`
echo $RUNS

#########################################
# loop over all runs
#########################################
for R in $RUNS
do
#########################################
# set correct epoch (V4, V5, V6)
   if [ $R -lt 46642 ]
   then
     EPOCH="V4"
   elif [ $R -gt 63373 ]
   then
     EPOCH="V6"
   else
     EPOCH="V5"
   fi
      
   echo "* $R $R 0 $CUTFILE $BM ${EFFAREA/EPOCHSETTING/$EPOCH} $BMPARA ${RADACC/EPOCHSETTING/$EPOCH}" >> $ODIR/$LLIST
done


#########################################
# submit the job
$EVNDISPSYS/scripts/VTS/VTS.ANASUM.sub_analyseParallel_data.sh $ODIR/$LLIST $DDIR $ODIR $RUNP

exit

