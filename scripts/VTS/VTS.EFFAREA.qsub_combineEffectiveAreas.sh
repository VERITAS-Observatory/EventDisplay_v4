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

###################################################################
# combine effective areas
$EVNDISPSYS/bin/combineEffectiveAreas "$DDIR" $TMPDIR/$OFIL false

mkdir -p $ODIR
mv -f $TMPDIR/$OFIL* $ODIR/

exit
