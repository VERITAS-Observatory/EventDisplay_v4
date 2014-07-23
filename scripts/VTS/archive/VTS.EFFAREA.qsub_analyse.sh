#$ -S /bin/tcsh
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
rm -f $ODIR/$EFF.root 
$EVNDISPSYS/bin/makeEffectiveArea $IFI $ODIR/$EFF.root > $ODIR/$EFF.log

exit
