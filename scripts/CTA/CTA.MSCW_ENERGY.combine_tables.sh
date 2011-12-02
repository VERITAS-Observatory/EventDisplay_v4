#!/bin/sh
#
# combine tables for CTA
#
# Author: Gernot Maier
#
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]
then
   echo
   echo "CTA.MSCW_ENERGY.combine_tables.sh <table file name> <subarray list> <output directory> "
   echo ""
   echo "  <table file name>  name of the table file (without .root)"
   echo
   echo "  <subarray list>    text file with list of subarray IDs"
   echo
   echo " <output directory>  directory for combined tables"
   echo
   echo " input data and output directories for tables are hard wired"
   exit
fi


# input parameters
TFIL=$1
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $2`

ODIR=$3
mkdir -p $ODIR

#loop over all arrays
for ARRAY in $VARRAY
do
   echo "combining tables for array $ARRAY"

#input data dir
   DDIR=$CTA_USER_DATA_DIR/analysis/$ARRAY/Tables/

#temporary list file
   LISTF="table.list.temp"

   rm -f $LISTF
   ls -1 $DDIR/*5.root > $LISTF
   wc -l $LISTF

#check if combine table exist - remove it (!)
   rm -f -v $ODIR/$TFIL.root

#combining files
   $EVNDISPSYS/bin/combineLookupTables $LISTF $ODIR/$TFIL-$ARRAY.root

   rm -f $LISTF

done

exit
