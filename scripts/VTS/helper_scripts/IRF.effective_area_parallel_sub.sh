#!/bin/bash
# script to calculate effective areas (VERITAS)

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
RUNPARAMS=RUNPFILE
OFILE=EAFILENAME
ODIR=OUTPUTDIR

# calculate effective areas
rm -f $ODIR/$OFILE.root 
$EVNDISPSYS/bin/makeEffectiveArea $RUNPARAMS $ODIR/$OFILE.root &> $ODIR/$OFILE.log

exit
