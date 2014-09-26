#!/bin/bash
# script to BDTs with TMVA
# Author: Maria Krause

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
RXPAR=RUNPARAM
          
# temporary (scratch) directory
#TEMPDIR=$TMPDIR/TMVA/
#mkdir -p $TEMPDIR

###rm -f $RPARA.log
$EVNDISPSYS/bin/trainTMVAforGammaHadronSeparation $RXPAR.runparameter > $RXPAR.log

exit
