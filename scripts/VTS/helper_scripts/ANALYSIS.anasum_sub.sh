#!/bin/bash
# script to analyse files with anasum

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
FLIST=FILELIST
INDIR=DATADIR
ODIR=OUTDIR
ONAME=OUTNAME
RUNP=RUNPARAM
RUNNUM=RUNNNNN

# run anasum
OUTPUTDATAFILE="$ODIR/$ONAME.root"
OUTPUTLOGFILE="$ODIR/$ONAME.log"
$EVNDISPSYS/bin/anasum   \
    -f $RUNP             \
    -l $FLIST            \
    -d $INDIR            \
    -o $OUTPUTDATAFILE   &> $OUTPUTLOGFILE
echo "RUN$RUNNUM ANPARLOG $OUTPUTLOGFILE"
echo "RUN$RUNNUM ANPARDATA $OUTPUTDATAFILE"

exit
