#!/bin/tcsh
#
# script to analyse files with anasum
#
# Author: Gernot Maier
#

set FLIST=FILELIST
set DDIR=DATADIR
set ODIR=OUTDIR
set ONAM=OUTNAM
set RUNP=RUNPARA

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh VERITAS

$EVNDISPSYS/bin/anasum -f $RUNP -l $FLIST -d $DDIR -o $ODIR/$ONAM.root >! $ODIR/$ONAM.log

exit

