#!/bin/bash
#
# script to write WP Phys Files
#
# Revision $Id$
#
# Author: Gernot Maier
#
#######################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ]
then
   echo "./CTA.WPPhysWriter.sh <sub array list> <directory with effective areas> <observation time [h]> <output file name>"
   echo
   echo "  <sub array list>          text file with list of subarray IDs"
   echo ""
   exit
fi

SUBAR=$1
DDIR=$2
OBSTIME=$3
OUTNAME=$4

VARRAY=`awk '{printf "%s ",$0} END {print ""}' $SUBAR`
for ARRAY in $VARRAY
do
   $EVNDISPSYS/bin/writeCTAWPPhysSensitivityFiles $ARRAY $OBSTIME $DDIR 1D $OUTNAME
done

############################################################################

exit
