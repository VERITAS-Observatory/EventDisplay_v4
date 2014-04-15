#!/bin/bash
#
# script to analyse data files with anasum (parallel analysis)
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ] 
then
   echo
   echo "VTS.ANASUM.sub_analyseParallel_data.sh <anasum run list> <data directory with mscw files> <output directory> <run parameter file>"
   echo
   echo "   <anasum run list>     full anasum run list (with effective areas file names, etc)"
   echo 
   echo "   <output directory>    anasum output files are written to this directory"
   echo
   echo "   <run parameter file>  anasum run parameter file"
   echo "                         (example can be found in $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/ANASUM.runparameter.dat)"
   echo
   exit
fi

FLIST=$1
DDIR=$2
ODIR=$3
RUNP=$4

# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi
if [ ! -f "$FLIST" ] ; then
    echo "Error, anasum runlist '$FLIST' not found, exiting..."
    exit 1
fi

# create extra stdout for duplication of command output
# look for ">&5" below
exec 5>&1

#########################################
# directory for run scripts
DATE=`date +"%y%m%d"`
LDIR=$VERITAS_USER_LOG_DIR"/queueAnasum/"$DATE"/"
mkdir -p $LDIR

# output directory
mkdir -p $ODIR

FNAM="$LDIR/ANA.$ONAM"

# temporary run list
DATECODE=`date +%Y%m%d`
TLIST=`basename $FLIST`
TLIST="$LDIR/$DATECODE.PID$$.$TLIST.tmp"
rm -f $TLIST
cat $FLIST | grep "*" >> $TLIST

LISTAD=`basename $ODIR`
echo "LISTAD $LISTAD"

# get list of runs
NLINES=`cat $TLIST |  wc -l `
NRUNS=`cat $TLIST | grep -v "VERSION" | wc -l `
echo "total number of runs to analyse: $NRUNS"

# loop over all runs
for ((i=1; i <= $NLINES; i++))
do
    LIN=`head -n $i $TLIST | tail -n 1`
    RUN=`head -n $i $TLIST | tail -n 1 | awk '{print $2}'`

    if [ $RUN != "VERSION" ]
    then

# output file name
       ONAM="$RUN.anasum"

# temporary file list
       TEMPLIST="${LDIR}/qsub_analyse_fileList_${LISTAD}_${RUN}_${DATECODE}_PID$$"
       rm -f $TEMPLIST
       echo "$LIN" > $TEMPLIST

# prepare run scripts
       FSCRIPT="$LDIR/qsub_analyse-"$DATE"-RUN$RUN"
       echo "run scripts written to $FSCRIPT"
       echo "temporary run list written to $TEMPLIST"

       sed -e "s|FILELIST|$TEMPLIST|" \
           -e "s|DATADIR|$DDIR|" \
           -e "s|OUTDIR|$ODIR|" \
           -e "s|OUTNAM|$ONAM|" \
           -e "s|RUNPARA|$RUNP|" VTS.ANASUM.qsub_analyse_data.sh > $FSCRIPT.sh

       chmod u+x $FSCRIPT.sh

		QSUBDATA=$( qsub -l os=sl6 -V -l h_cpu=12:29:30 -l h_vmem=4000M -l tmpdir_size=1G -o $LDIR/ -e $LDIR/ "$FSCRIPT.sh"  | tee >(cat - >&5) )
		JOBID=$( echo "$QSUBDATA" | grep -E "Your job" | awk '{ print $3 }' )
		echo "RUN$RUN SCRIPT $FSCRIPT.sh"
		echo "RUN$RUN JOBID $JOBID"
		echo "RUN$RUN ELOG $FSCRIPT.sh.e$JOBID"
		echo "RUN$RUN OLOG $FSCRIPT.sh.o$JOBID"

    fi
done 

rm -f $TLIST

exit
