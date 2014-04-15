#!/bin/sh
#
# VTS.checkRuns.sh
# Small scritp that quickly check the ED and mscw log files for errors and warnings.
# Also prints out the ped values - quick check for missing flasher info.
#
# Gareth
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] 
then
   echo "checkRuns.sh < runlist > < ED Version > < Recon ID >"
   exit
fi

RUNLIST=$1
VERSION=$2
CUT=$3

DATE=20130524

FILE=`cat $RUNLIST`
for AFIL in $FILE
do

 LOGFILE="${VERITAS_USER_LOG_DIR}/analysis/EVD${VERSION}-d$DATE/${AFIL}.log"
 MSCWFILE="${VERITAS_USER_DATA_DIR}/analysis/EVD${VERSION}-d$DATE/RecID${CUT}/${AFIL}.mscw.log"

 if [ -n "$3" ]
 then
   echo $MSCWFILE
   grep -iH error $MSCWFILE 
   grep -iH warn  $MSCWFILE
   grep -H ped $MSCWFILE
 else
   grep -iH exit $LOGFILE
   grep -iH error $LOGFILE
   grep -iH warn  $LOGFILE
 fi

done

exit
