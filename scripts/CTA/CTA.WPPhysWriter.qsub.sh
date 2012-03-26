#!/bin/tcsh
#
# script to write WP Phys Files
#
#
# Author: Gernot Maier
#
#######################################################################

set AXRRAY=ARRAY
set DXDIR=DDIR
set OXBSTIME=OBSTIME
set OXUTNAME=OUTNAME
set OXFFSET=OFFSET

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

rm -f $OXUTNAME-$OXBSTIME"h."$AXRRAY.log

$EVNDISPSYS/bin/writeCTAWPPhysSensitivityFiles $AXRRAY $OXBSTIME $DXDIR $OXUTNAME CTA $OXFFSET > $OXUTNAME-$OXBSTIME-$AXRRAY.log

############################################################################

exit
