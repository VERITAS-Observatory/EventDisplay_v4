#!/bin/sh
#
# script run eventdisplay laser analysis with a queue system
#
# Author: Gernot Maier
#

if [ ! -n "$1" ]
then
   echo
   echo "Laser analysis: submit jobs from a simple run list"
   echo 
   echo "VTS.EVNDISP.sub_analyse_laser.sh <runlist> "
   echo
   echo "runlist should contain laser run numbers"
   echo
   echo "example for run list:"
   echo "48626"
   echo "58453"
   echo "61429"
   echo
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
echo "Laser files to analyse:"
echo "$FILES"

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
# check if laser file exists

   DFIL=`find -L $VERITAS_DATA_DIR/data/ -name "$AFIL.cvbf"`
   if [ -z "$DFIL" ]
   then
     echo "Error: laser vbf file not found for run $AFIL"
   else
      echo "now running $AFIL"
      FNAM="$SHELLDIR/EVN.laser-$AFIL"

      sed -e "s/RRRRR/$AFIL/" $FSCRIPT.sh > $FNAM.sh

      chmod u+x $FNAM.sh
      echo $FNAM.sh

      qsub -V  -l os=sl6 -l h_cpu=11:29:00 -l h_vmem=2000M -l tmpdir_size=5G -o $QLOG/ -e $QLOG/ "$FNAM.sh"

      sleep 1s
   fi
done

exit

