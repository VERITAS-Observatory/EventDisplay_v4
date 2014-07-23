#!/bin/bash
#
# script to analyse VTS data files with lookup tables
#
# Author: Gernot Maier
#

if [ $# -ne 3 ] && [ $# -ne 4 ]
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
DATE=`date +"%y%m%d"`
QLOG=$VERITAS_USER_LOG_DIR/$DATE/MSCW.ANADATA
mkdir -p $QLOG

# create extra stdout for duplication of command output
# look for ">&5" below
exec 5>&1

# skeleton script
FSCRIPT="VTS.MSCW_ENERGY.qsub_analyse_data"

#########################################
# loop over all files in files loop
for AFIL in $FILES
do
   BFIL=$EFIL/$AFIL.root
   echo "now analysing $BFIL (ID=$ID)"

   FNAM="$QLOG/MSCW.data-ID$ID-$AFIL"
   rm -f $FNAM.sh

   sed -e "s|TABLEFILE|$TFIL|" \
       -e "s|RECONSTRUCTIONID|$ID|" \
       -e "s|EVNDFIL|$BFIL|" $FSCRIPT.sh > $FNAM.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

	QSUBDATA=$( qsub -S /bin/tcsh -l os=sl6 -l h_cpu=00:29:00 -l h_vmem=2000M -l tmpdir_size=4G -V -o $QLOG -e $QLOG "$FNAM.sh"  | tee >(cat ->&5) )
	JOBID=$( echo "$QSUBDATA" | grep -E "Your job" | awk '{ print $3 }' )

	# tell the user basic info about the job submission
	echo "RUN$AFIL SCRIPT $FNAM.sh"
	echo "RUN$AFIL JOBID $JOBID"
	echo "RUN$AFIL OLOG $FNAM.sh.o$JOBID"
	echo "RUN$AFIL ELOG $FNAM.sh.e$JOBID"
	echo


done

exit
