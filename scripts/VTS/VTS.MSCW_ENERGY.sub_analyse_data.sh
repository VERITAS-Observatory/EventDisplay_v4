#!/bin/sh
#
# script to analyse VTS data files with lookup tables
#
# Author: Gernot Maier
#

if [ $# -ne 3 ]
then
   echo "VTS.MSCW_ENERGY.sub_analyse_data.sh <table file> <directory of evndisp files> <list of run> [ID]"
   echo
   echo "   <table file>  table file name (without .root)"
   echo
   echo "   [ID]          reconstruction ID (default=0)"
   echo
   exit
fi

#########################################
# input variables
TFIL=$1
EFIL=$2
BLIST=$3
ID=0
if [ -n "$4" ]
then
  ID=$4
fi

# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

#########################################
# get list of runs
FILES=`cat $BLIST`

#########################################
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`
QLOG=$VERITAS_USER_LOG_DIR/$DATE/
mkdir -p $QLOG
QLOG="/dev/null"

# output directory for shell scripts
SHELLDIR=$VERITAS_USER_LOG_DIR"/queueShellDir/"
mkdir -p $SHELLDIR

# skeleton script
FSCRIPT="VTS.MSCW_ENERGY.qsub_analyse_data"

#########################################
# loop over all files in files loop
for AFIL in $FILES
do
   BFIL=$EFIL/$AFIL.root
   echo "now analysing $BFIL (ID=$ID)"

   FNAM="$SHELLDIR/MSCW.data-ID$ID-$AFIL"

   sed -e "s|TABLEFILE|$TFIL|" $FSCRIPT.sh > $FNAM-1.sh
   sed -e "s|RECONSTRUCTIONID|$ID|" $FNAM-1.sh > $FNAM-2.sh
   rm -f $FNAM-1.sh
   sed -e "s|EVNDFIL|$BFIL|" $FNAM-2.sh > $FNAM.sh
   rm -f $FNAM-2.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

   qsub -l h_cpu=00:29:00 -l h_vmem=2000M -l tmpdir_size=4G -V -o $QLOG -e $QLOG "$FNAM.sh"
done

exit
