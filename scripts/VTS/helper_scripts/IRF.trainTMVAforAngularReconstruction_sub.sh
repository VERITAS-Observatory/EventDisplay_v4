#!/bin/bash
# script to train TMVA (BDTs) for angular reconstruction

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
INDIR=EVNDISPFILE
ODIR=OUTPUTDIR
ONAME=BDTFILE

# train
rm -f $ODIR/$ONAME*

$EVNDISPSYS/bin/trainTMVAforAngularReconstruction "$INDIR/*[0-9].root" $ODIR $ONAME > $ODIR/$ONAME.log

exit
