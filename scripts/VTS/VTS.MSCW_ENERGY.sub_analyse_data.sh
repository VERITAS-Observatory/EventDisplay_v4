#!/bin/sh
#
# script to analyse data files with lookup tables
#
# Revision $Id: analyse.sh,v 1.1.2.2.6.1.2.1.8.2.2.6.2.2.6.1 2011/03/22 08:27:33 gmaier Exp $
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] && [ ! -n "$2" ]  && [ ! -n "$3" ]  && [ ! -n "$4" ]
then
   echo "VTS.MSCW_ENERGY.sub_analyse_data.sh <table file> <directory of evndisp files> <list of run> <rec id>"
   echo
   echo "   <table file>  table file name (without .root)"
   echo "   <rec ID>      EVNDISP reconstruction parameter"
   exit
fi

#########################################
# input variables
TFIL=$1
EFIL=$2
BLIST=$3
ID=$4

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

   qsub -l h_cpu=00:20:00 -l h_vmem=2000M -l tmpdir_size=4G -V -o $QDIR -e $QDIR "$FNAM.sh"
done

exit
