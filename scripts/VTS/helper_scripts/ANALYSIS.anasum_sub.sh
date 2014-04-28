#!/bin/bash
# script to analyse files with anasum
# Author: Gernot Maier

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
FLIST=FILELIST
INDIR=DATADIR
ODIR=OUTDIR
ONAME=OUTNAME
RUNP=RUNPARAM

# temporary (scratch) directory
TEMPDIR=$TMPDIR/ANASUM/
mkdir -p $TEMPDIR

# run anasum
$EVNDISPSYS/bin/anasum   \
    -f $RUNP             \
    -l $FLIST            \
    -d $INDIR            \
    -o $ODIR/$ONAME.root \
    &> $ODIR/$ONAME.log
echo "RUN`basename $ONAME .anasum` ANPARLOG $ODIR/$ONAME.log"
echo "RUN`basename $ONAME .anasum` DATAOUT $ODIR/$ONAME.root"

exit
