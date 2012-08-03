#!/bin/sh
#
# Download Data from VTS Database
#
# Author: Gareth Hughes
#

if [ ! -n "$1" ]
then
   echo
   echo "VTS.EVNDISP.getDataFromDB.sh  <db start time> <db stop time> <Source ID> <Minimum Elevation> <Minimum Run Duration [s]> <Find Laser Runs (1/0)> <Download Files (1/0)>" 
   echo
   exit
fi

source $EVNDISPSYS/setObservatory.sh VERITAS

$EVNDISPSYS/bin/VTS.getRunListFromDB $1 $2 $3 $4 $5 $6 $7

exit
