#!/bin/bash
#
# script to link gammas and electron files from one data set to another
#
########################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ]
then
     echo 
     echo "./CTA.LinkFilesFromProdSets.sh <sub array list> <AnalysisDirectory> <Loc 1> <Loc 2> "
     echo
     echo "  <sub array list>          text file with list of subarray IDs"
     echo
     exit
fi

SUBAR=$1
ANADIR=$2

###############################################################
# loop over all arrays
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $1`
for ARRAY in $VARRAY
do
   echo "STARTING ARRAY $ARRAY"

   SDIR="$CTA_USER_DATA_DIR/analysis/AnalysisData/$3/$ARRAY/$ANADIR/"
   ODIR="$CTA_USER_DATA_DIR/analysis/AnalysisData/$4/$ARRAY/$ANADIR/"

# make sure that upper directory exists
   mkdir -p $ODIR

   echo $SDIR
   echo $ODIR
#   ln -f -s $SDIR/gamma* $ODIR/
#   ln -f -s $SDIR/elec* $ODIR/
   ln -f -s $SDIR/prot* $ODIR/
done

exit

