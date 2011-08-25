#!/bin/sh
#
# script run eventdisplay with a queue system
#
# Revision $Id: mm_sub_evndisp.sh,v 1.1.2.1.4.1.12.3.2.8.2.3 2011/01/03 08:22:27 gmaier Exp $
#
#
# Author: Gernot Maier
#

if [ ! -n "$1" ]
then
   echo "VTS.EVNDISP.sub_analyse_data.sh <runlist> [sumwindow (default=12)] [pedestal calculation (default=1=on)] [method=GEO/LL (default=LL)]"
   exit
fi

RLIST=$1
SW=12
PED=1
MET=LL
if [ -n "$2" ]
then
  SW=$2
fi
if [ -n "$3" ]
then
  PED=$3
fi
if [ -n "$4" ]
then
  MET=$4
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
   sed -e "s|SUMWINDOW|$SW|" $FNAM-1.sh > $FNAM-2.sh
   rm -f $FNAM-1.sh
   sed -e "s|MEEET|$MET|" $FNAM-2.sh > $FNAM-3.sh
   rm -f $FNAM-2.sh
   sed -e "s|PEEED|$PED|" $FNAM-3.sh > $FNAM.sh
   rm -f $FNAM-3.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

# GEO is much faster than LL
   if [ $MET = "LL" ]
   then
      qsub -V -l h_cpu=04:00:00 -l h_vmem=2000M -l tmpdir_size=10G -o $QLOG/ -e $QLOG/ "$FNAM.sh"
   fi
   if [ $MET = "GEO" ]
   then
      qsub -V -l h_cpu=01:30:00 -l h_vmem=2000M -l tmpdir_size=10G -o $QLOG/ -e $QLOG/ "$FNAM.sh"
   fi
done

exit

