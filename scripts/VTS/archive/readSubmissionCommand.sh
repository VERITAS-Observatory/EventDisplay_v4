#!/bin/sh
#
# script to read submission command from a parameter file
#
#############################################################

if [ $# -lt 2 ]
then
      echo "./readSubmissionCommand.sh <file with commands> <analysis stage>"
      exit
fi

if [ ! -e $1 ]
then
   echo "ERROR"
   exit
fi

# get command
COMA=`grep -v "\#" $1 | grep $2`
FCOM=`grep -v "\#" $1 | grep $2 | awk '{print $2}'`

# not found - return error
if [ -z "$COMA" ]
then
   echo "ERROR"
   exit
fi

# check if this is a qsub command
if [[ $FCOM == "QSUB" ]]
then
    echo ${COMA##*$2}
else
    echo $FCOM
fi


exit

