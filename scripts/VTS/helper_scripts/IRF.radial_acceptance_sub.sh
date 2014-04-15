#!/bin/bash
# generate a radial acceptance file
# Author: Gernot Maier

# parameters replaced by parent script using sed
RLIST=RUNLIST
DDIR=INPUTDIR
CUTS=CUTSFILE
ODIR=OUTPUTDIR
OFILE=OUTPUTFILE

# create radial acceptance
rm -f $ODIR/$OFILE.log
$EVNDISPSYS/bin/makeRadialAcceptance -s $RLIST -c $CUTS -d $DDIR -o $ODIR/$OFILE.root &> $ODIR/$OFILE.log

exit
