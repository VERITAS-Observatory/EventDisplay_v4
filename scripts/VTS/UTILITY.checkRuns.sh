#!/bin/bash
#
# VTS.checkRuns.sh
# Small script that quickly check the ED and mscw log files for errors and warnings.
# Also prints out the ped values - quick check for missing flasher info.
#
# Gareth
#
# NOTE: THIS SCRIPT DOESN'T REALLY WORK YET (SG)


if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
    echo "checkRuns.sh <runlist> <Rec ID>"
    exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
RUNLIST=$1
RECID=$2

DATE=20130524

FILE=`cat $RUNLIST`
for AFIL in $FILE
do
    LOGFILE="${VERITAS_USER_LOG_DIR}/analysis/EVD${EDVERSION}-d$DATE/${AFIL}.log"
    MSCWFILE="${VERITAS_USER_DATA_DIR}/analysis/EVD${EDVERSION}-d$DATE/RecID${RECID}/${AFIL}.mscw.log"

    if [ -n "$3" ]; then
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
