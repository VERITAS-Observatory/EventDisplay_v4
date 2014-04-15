#!/bin/bash
# script to analyse laser files 
# Author: Gernot Maier

# parameters replaced by parent script using sed
RUN=RUNFILE
LOGDIR=LOGDIRECTORY
echo $RUN

# run eventdisplay
rm -f $LOGDIR/$RUN.laser.log
$EVNDISPSYS/scripts/VTS/SPANALYSIS.evndisp_laser_run.sh $RUN &> $LOGDIR/$RUN.laser.log

exit
