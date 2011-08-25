#!/bin/tcsh
#
# script to analyse files with anasum
#
# Revision $Id: qsub_analyse.sh,v 1.1.2.1.4.1.6.1 2011/04/06 11:25:32 gmaier Exp $
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

