#!/bin/bash
#
# simple script to get alias file names from GRID
#
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]
then
   echo "./CTA.getAliasesFromGrid.sh <run list> <target directory> <glite command (0=no/1=yes)>"
   echo
   exit
fi

# loop over all files in the list
FILEL=`cat $1`
for i in $FILEL
do
    OFIL=`basename $i`
    if [ -e $2/$OFIL ] && [ -s $2/$OFIL ]
    then
       echo "FILE EXISTS: $2/$OFIL"
    else
       surl=`lcg-lr lfn:/grid$i`
       if [ $3 -eq "0" ]
       then
          echo $surl
       else
          echo "$surl srm://styx.ifh.de:8443/srm/v2/server?SFN=$2/${FIL}"
       fi
    fi
done

exit
