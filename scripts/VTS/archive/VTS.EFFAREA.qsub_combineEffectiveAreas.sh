#$ -S /bin/tcsh
#
# script to combine effective areas
#
# Author: Gernot Maier
#

cd $EVNDISPSYS
source ./setObservatory.tcsh VTS

###################################################################
# parameters set by parent scripts
###################################################################
# input directory
set DDIR="DDDD"
# output file 
set OFIL=OOOO
# output directory
set ODIR=ODOD
#
mkdir -p $ODIR

###################################################################
# combine effective areas
$EVNDISPSYS/bin/combineEffectiveAreas "$DDIR" $ODIR/$OFIL false >& $ODIR/$OFIL.combine.log 

exit
