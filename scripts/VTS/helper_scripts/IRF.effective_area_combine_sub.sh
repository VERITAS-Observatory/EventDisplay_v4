#!/bin/bash
# script to combine effective areas
# Author: Gernot Maier

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
EAFILES=INPUTFILES
OFILE=OUTPUTFILE
ODIR=OUTPUTDIR
mkdir -p $ODIR

# Write histograms?
WRITEHISTOS="false"

# combine effective areas
$EVNDISPSYS/bin/combineEffectiveAreas "$EAFILES" $ODIR/$OFILE $WRITEHISTOS &> $ODIR/$OFILE.log 

exit
