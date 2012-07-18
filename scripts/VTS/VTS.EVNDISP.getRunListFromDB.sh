#!/bin/sh
#
# Download Data from VTS Database
#
# Author: Gareth Hughes
#

if [ ! -n "$1" ]
then
   echo "VTS.EVNDISP.getDataFromDB.sh  <db start time> <db stop time> <Source ID> <Minimum Elevation> <Minimum Run Duration [s]> <Find Laser Runs (1/0)> <Download Files (1/0)>" 
   exit
fi

source $EVNDISPSYS/trunk/setObservatory.sh VERITAS

cd $EVNDISPSYS/trunk/bin/

./VTS.getRunListFromDB $1 $2 $3 $4 $5 $6 $7
