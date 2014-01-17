#!/bin/bash
#
# simple script to download raw files from the GRID
#
# (you have to add the password manually)
#

if [ ! -n "$1" ] && [ ! -n "$2" ]
then
   echo "./CTA.getAliasesFromGrid.sh <run list> <target directory>"
   echo
   exit
fi

# dcache client
export DCACHE_CLIENT_ACTIVE=1

# loop over all files in the list
FILEL=`cat $1`
for i in $FILEL
do
    OFIL=`basename $i`
    if [ -e $2/$OFIL ] && [ -s $2/$OFIL ]
    then
       echo "FILE EXISTS: $2/$OFIL"
    else
       lcg-lr lfn:/grid$i
       sleep 1
    fi
done

exit
