#!/bin/bash
#
# script to link gammas and electron files from one data set to another
# (link here north and south)
#
########################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ]
then
     echo 
     echo "./CTA.LinkFilesFromProdSets.sh <sub array list> <site>"
     echo
     echo "   links north/south data set into -NS data set"
     echo
     exit
fi

SUBAR=$1
SITE=$2

PARTICLE=( "gamma_onSource" "gamma_cone10" "electron" "proton" )

###############################################################
# loop over all arrays
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $1`
for ARRAY in $VARRAY
do
   echo "STARTING ARRAY $ARRAY"

   for ((i = 0; i < ${#PARTICLE[@]}; i++ ))
   do
      PART=${PARTICLE[$i]}
      echo "   PARTICLE $PART"

      ODIR="$CTA_USER_DATA_DIR/analysis/AnalysisData/$SITE-NS/$ARRAY/$PART/"
      mkdir -p $ODIR

      for N in North South
      do
         echo "      $N"

	 SDIR="$CTA_USER_DATA_DIR/analysis/AnalysisData/$SITE-$N/$ARRAY/$PART/"
	 ln -s $SDIR/*.root $ODIR/
      done
   done
done

exit

