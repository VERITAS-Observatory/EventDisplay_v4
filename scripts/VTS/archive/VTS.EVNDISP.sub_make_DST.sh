#!/bin/sh
#
# script to make DST files from a raw data file
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]
then
   echo
   echo "EVNDISP DST maker: submit jobs from a simple run list"
   echo
   echo "VTS.EVNDISP.sub_make_DST.sh <runlist> [pedestal calculation (default=1=on)] [Summation Window]" 
   echo
   echo "runlist should contain run numbers only"
   echo
   echo "example for run list:"
   echo "48626"
   echo "58453"
   echo "61429"
   echo

   exit
fi

RLIST=$1
PED=1
if [ -n "$2" ]
then
  PED=$2
fi
SUMW=$3

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
FSCRIPT="VTS.EVNDISP.qsub_make_DST_SW"

#########################################
# loop over all files in files loop
for AFIL in $FILES
do
   echo "now running $AFIL"
   FNAM="$SHELLDIR/EVN.DST-$AFIL-$SUMW"

   sed -e "s|RRRRR|$AFIL|" $FSCRIPT.sh > $FNAM-1.sh
   sed -e "s|PEEED|$PED|" $FNAM-1.sh > $FNAM-2.sh
   sed -e "s|SWSW|$SUMW|" $FNAM-2.sh > $FNAM.sh
   rm -f $FNAM-1.sh
   rm -f $FNAM-2.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

   qsub -V -l h_cpu=11:29:00 -l os="sl*" -l h_vmem=4000M -l tmpdir_size=40G -o $QLOG/ -e $QLOG/ "$FNAM.sh"
done

exit

