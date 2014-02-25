#!/bin/bash
#
# simple script to get alias file names from GRID
#
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ]
then
   echo "./CTA.getAliasesFromGrid.sh <run list> <data directory> <glite command (0=no/1=yes)> <output list file directory>"
   echo
   exit
fi

# dcache client
export DCACHE_CLIENT_ACTIVE=1

# output list file directory
mkdir -p $4
TLIS=`basename $1`
FILEEX="$4/$TLIS.exist"
touch $FILEEX
FILELO="$4/$TLIS.dcache"
touch $FILELO
FILEGL="$4/$TLIS.glite"
touch $FILEGL

# loop over all files in the list
FILEL=`cat $1`
for i in $FILEL
do
    OFIL=`basename $i`
# check if file is already in the target directory
    if [ -e $2/$OFIL ] && [ -s $2/$OFIL ]
    then
       echo "FILE EXISTS: $2/$OFIL" >> $FILEEX
# check if the file is on the local dCache
    else
       DC="/acs/grid/cta/$i"
       if [ -e $DC ]
       then
          echo "dccp $DC $2/$OFIL" >> $FILELO
       else
          surl=`lcg-lr lfn:/grid$i`
          if [ $3 -eq "0" ]
          then
             echo $surl >> $FILEGL
          else
             echo "$surl srm://styx.ifh.de:8443/srm/v2/server?SFN=$2/${OFIL}" >> $FILEGL
          fi
       fi
    fi
done

exit
