#!/bin/bash
# generate a radial acceptance file

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
RLIST=RUNLIST
DDIR=INPUTDIR
CUTS=CUTSFILE
ODIR=OUTPUTDIR
OFILE=OUTPUTFILE
EPOCH=IEPO

# create radial acceptance
rm -f $ODIR/$OFILE.log
$EVNDISPSYS/bin/makeRadialAcceptance -s $RLIST -c $CUTS -d $DDIR -i $EPOCH -o $ODIR/$OFILE.root &> $ODIR/$OFILE.log

exit
