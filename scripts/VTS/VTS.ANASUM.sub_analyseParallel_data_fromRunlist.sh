#!/bin/sh
#
# script to analyse data files with anasum (parallel analysis) from a simple run list
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ] 
then
   echo
   echo "VTS.ANASUM.sub_analyseParallel_data.sh <run list> <data directory with mscw files> <cut set> <output directory for anasum products> <run parameter file>"
   echo
   echo "   <run list>     simple run list "
   echo 
   echo "   <cut set>      cut sets predifined and hardwired in the script:"
   echo "                  soft, moderate"
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
CUTS=$3
ODIR=$4
RUNP=$5

#########################################
# cut definitions
if [[ "$CUTS" == *soft* ]]
then
   CUTFILE="ANASUM.GammaHadron.d20120909-cut-N2-Point-005CU-Soft.dat"
   EFFAREA="effArea-d20120909-cut-N2-Point-005CU-Soft-EPOCHSETTING-d20121218.root"
   RADACC="radialAcceptance-d20120909-cut-N2-Point-005CU-Soft-EPOCHSETTING-d20121218.root"
elif [[ $CUTS = *moderate* ]]
then
   CUTFILE="ANASUM.GammaHadron.d20120909-cut-N3-Point-005CU-Moderate.dat"
   EFFAREA="effArea-d20120909-cut-N3-Point-005CU-Moderate-EPOCHSETTING-d20121218.root"
   RADACC="radialAcceptance-d20120909-cut-N3-Point-005CU-Moderate-EPOCHSETTING-d20121218.root"
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
   BMPARA="0.1 1 20"
elif [[ "$CUTS" == *RB* ]]
then
   BM="RB"
   BMPARA="0.6 20"
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

for R in $RUNS
do
# get epoch 
   if [ $R -lt 46642 ]
   then
     EPOCH="V4"
   elif [ $R -gt 63408 ]
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

