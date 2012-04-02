#!/bin/tcsh
#
# Revision $Id: qsub_getEffectiveArea.sh,v 1.1.2.1.8.2.6.1.2.3.2.3.2.2.6.1 2011/04/21 06:39:21 gmaier Exp $
#
# script to calculate effective areas (VERITAS)
#
# Author: Gernot Maier
#

cd $EVNDISPSYS
source ./setObservatory.tcsh VTS

###################################################################
# parameters set by parent scripts
###################################################################
# parameter file
set IFI=MSCWFILE
# output file (with effective areas)
set EFF=EFFFILE
# output directory
set ODIR=OOOOOOO
###################################################################
mkdir -p $ODIR

###################################################################
# calculate effective areas
rm -f $ODIR/$EFF.root $LDIR/$EFF.log
$EVNDISPSYS/bin/makeEffectiveArea $IFI $ODIR/$EFF.root > $ODIR/$EFF.log

exit
