#!/bin/sh
#
# script run eventdisplay analysis for VTS data 
#
# Author: Gernot Maier
#

if [ ! -n "$1" ]
then
   echo
   echo "EVNDISP data analysis: submit jobs from a simple run list"
   echo
   echo "VTS.EVNDISP.sub_analyse_data.sh <runlist> [calibration (default=1)] [VPM (default=1)] [output directory]" 
   echo
   echo "  [calibration]"
   echo "          1     pedestal & average tzero calculation (default)"
   echo "          2     pedestal calculation only"
   echo "          3     average tzero calculation only"
   echo
   echo "  runlist should contain run numbers only"
   echo
   echo "  example for run list:"
   echo "    48626"
   echo "    58453"
   echo "    61429"
   echo

   exit
fi

RLIST=$1
PED=1
if [ -n "$2" ]
then
  PED=$2
fi
VPM=1
if [ -n "$3" ]
then
  VPM=$3
fi
ODIR="$VERITAS_USER_DATA_DIR"/analysis/Results/EVD-v423/
if [ -n "$4" ]
then
   ODIR=$4
fi
mkdir -p $ODIR

##############################
# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

###############################################################################################################
FILES=`cat $RLIST`
echo $FILES
#########################################
# output directory for shell scripts
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`
QLOG=$VERITAS_USER_LOG_DIR/$DATE/EVNDISP.ANADATA
mkdir -p $QLOG

# skeleton script
FSCRIPT="VTS.EVNDISP.qsub_analyse_data"

#########################################
# loop over all files in files loop
for AFIL in $FILES
do
   echo "now starting run $AFIL"
   FNAM="$QLOG/EVN.data-$AFIL"

   sed -e "s|RRRRR|$AFIL|" \
       -e "s|PEEED|$PED|" \
       -e "s|OODIR|$ODIR|" \
       -e "s|VVPM|$VPM|" $FSCRIPT.sh > $FNAM.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

   qsub -V -l h_cpu=41:29:00 -l os="sl*" -l h_vmem=2000M -l tmpdir_size=10G -o $QLOG/ -e $QLOG/ "$FNAM.sh"
done

exit

