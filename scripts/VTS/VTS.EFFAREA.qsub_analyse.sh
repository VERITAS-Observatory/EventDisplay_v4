#!/bin/tcsh
#
# Revision $Id: qsub_getEffectiveArea.sh,v 1.1.2.1.8.2.6.1.2.3.2.3.2.2.6.1 2011/04/21 06:39:21 gmaier Exp $
#
# script to calculate effective areas (VERITAS)
#
# Author: Gernot Maier
#

###################################################################
# parameters set by parent scripts
###################################################################
# parameter file
set IFI=MSCWFILE
# output file (with effective areas)
set EFF=EFFFILE
# output directory
set ODIR=OOOOOOO
# log directory
set LDIR=LLLLLLL
###################################################################
mkdir -p $ODIR
mkdir -p $LDIR

###################################################################
# calculate effective areas
rm -f $ODIR/$EFF.root $LDIR/$EFF.log
$EVNDISPSYS400/bin/makeEffectiveArea $IFI $ODIR/$EFF.root > $LDIR/$EFF.log

exit
