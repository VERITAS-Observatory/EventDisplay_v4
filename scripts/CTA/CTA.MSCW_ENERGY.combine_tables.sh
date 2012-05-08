#!/bin/sh
#
# combine tables for CTA
#
# Author: Gernot Maier
#
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]  && [ ! -n "$4" ]  && [ ! -n "$5" ]
then
   echo
   echo "CTA.MSCW_ENERGY.combine_tables.sh <combined table file name> <subarray list> <input table file name> <output directory> <data set> "
   echo ""
   echo "  <combined table file name>  name of the table combined file (without .root)"
   echo
   echo "  <subarray list>             text file with list of subarray IDs"
   echo
   echo "  <input table file name>     name of the input table name (beginning of...)"
   echo
   echo "  <output directory>          directory for combined tables"
   echo
   echo "   input data and output directories for tables are hard wired to"
   echo "      \$CTA_USER_DATA_DIR/analysis/AnalysisData/<dataset>/\$ARRAY/Tables/"
   echo
   exit
fi


# input parameters
TFIL=$1
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $2`
ITFIL=$3
ODIR=$4
mkdir -p $ODIR
DSET=$5

#loop over all arrays
for ARRAY in $VARRAY
do
   echo "combining tables for array $ARRAY"

#input data dir
   DDIR=$CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/Tables/

#temporary list file
   LISTF="table.list.temp"

   rm -f $LISTF
   ls -1 $DDIR/$ITFIL*5-$ARRAY.root > $LISTF
   wc -l $LISTF

   echo "combining following files: " 
   cat $LISTF

#check if combine table exist - remove it (!)
   rm -f -v $ODIR/$TFIL.root

#combining files
   $EVNDISPSYS/bin/combineLookupTables $LISTF $ODIR/$TFIL-$ARRAY.root

   rm -f $LISTF

done

exit
