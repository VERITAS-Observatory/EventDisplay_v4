#!/bin/sh
#
# script run eventdisplay laser analysis with a queue system
#
# Revision $Id: mm_sub_evndisp_laser.sh,v 1.1.2.2.6.1 2011/03/17 11:01:13 gmaier Exp $
#
#
# Author: Gernot Maier
#

if [ ! -n "$1" ]
then
   echo "VTS.EVNDISP.sub_analyse_laser.sh <runlist> "
   echo
   echo "runlist should contain laser run numbers"
   exit
fi

RLIST=$1

# checking the path for binaries
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
FSCRIPT="VTS.EVNDISP.qsub_analyse_laser"

#########################################
# loop over all files in files loop
for AFIL in $FILES
do
   echo "now running $AFIL"
   FNAM="$SHELLDIR/EVN.laser-$AFIL"

   sed -e "s/RRRRR/$AFIL/" $FSCRIPT.sh > $FNAM.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

   qsub -V -l h_cpu=00:25:00 -l h_vmem=2000M -l tmpdir_size=5G -o $QLOG/ -e $QLOG/ "$FNAM.sh"
done

exit

