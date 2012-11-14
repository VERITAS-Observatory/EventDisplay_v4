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
   echo "VTS.EVNDISP.sub_analyse_data.sh <runlist> [calibration (default=1)]" 
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
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`
QLOG=$VERITAS_USER_LOG_DIR/$DATE/
mkdir -p $QLOG

# output directory for shell scripts
SHELLDIR=$VERITAS_USER_LOG_DIR"/queueShellDir/"
mkdir -p $SHELLDIR

# skeleton script
FSCRIPT="VTS.EVNDISP.qsub_analyse_data"

#########################################
# loop over all files in files loop
for AFIL in $FILES
do
   echo "now running $AFIL"
   FNAM="$SHELLDIR/EVN.data-$AFIL"

   sed -e "s|RRRRR|$AFIL|" $FSCRIPT.sh > $FNAM-1.sh
   sed -e "s|PEEED|$PED|" $FNAM-1.sh > $FNAM.sh
   rm -f $FNAM-1.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

   qsub -js 200 -V -l h_cpu=11:29:00 -l os="sl*" -l h_vmem=2000M -l tmpdir_size=10G -o $QLOG/ -e $QLOG/ "$FNAM.sh"
done

exit

