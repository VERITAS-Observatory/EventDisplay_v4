#!/bin/bash
# script to combine effective areas
# Author: Gernot Maier

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
