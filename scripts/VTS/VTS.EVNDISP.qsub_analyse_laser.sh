#!/bin/tcsh
#
# script to analyse laser files 
#
# Revision $Id: qsub_evndisp_laser.sh,v 1.1.2.1.6.1 2011/03/17 11:01:13 gmaier Exp $
#
# Author: Gernot Maier
#

set RUN=RRRRR
echo $RUN

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh VERITAS

# output data files are written to $VERITAS_EVNDISP_ANA_DIR/Calibration/Tel_XXX/
# output log files are written to this directory
set LDIR=$VERITAS_USER_LOG_DIR"/analysis/EVD400-LASER/"
mkdir -p $LDIR

# data file
set DDIR=$VERITAS_DATA_DIR"/data/"
set DFIL=`find -L $DDIR -name "$RUN.cvbf"`

#########################################
# run eventdisplay
rm -f $LDIR/$RUN.laser.log

$EVNDISPSYS/scripts/VTS/VTS.EVNDISP.analyse_laser_run -1 $DFIL >! $LDIR/$RUN.laser.log


exit

