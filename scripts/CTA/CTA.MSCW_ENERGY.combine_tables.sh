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
   echo "CTA.MSCW_ENERGY.combine_tables.sh <table file name> <array ID> <output directory> "
   echo ""
   echo "  <table file name>  name of the table file (without .root)"
   echo
   echo "  <array ID>         CTA array ID (e.g. E for array E)"
   echo "                     use ALL for all arrays (A B C D E F G H I J K NA NB)"
   echo
   echo " <output directory>  directory for combined tables"
   echo
   echo " input data and output directories for tables are hard wired"
   exit
fi


# input parameters
TFIL=$1
ARRAY=$2
if [ $ARRAY == "ALL" ]
then
  VARRAY=( A B C D E F G H I J K NA NB "s4-2-120" "s4-2-85" "s4-2-120" "I-noLST" "I-noSST" "g60" "g85" "g120" "g170" "g240" "s9-2-120" "s9-2-170" )
#  VARRAY=( "s2-1-75" "s3-1-210" "s3-3-260" "s3-3-346" "s3-4-240" "s4-1-105" "s4-2-170" "s4-3-200" "s4-4-140" "s4-4-150" "s4-5-125" )
else 
  VARRAY=( $ARRAY )
fi
NARRAY=${#VARRAY[@]}

ODIR=$3
mkdir -p $ODIR

#loop over all arrays
for (( N = 0 ; N < $NARRAY; N++ ))
do
   ARRAY=${VARRAY[$N]}
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
