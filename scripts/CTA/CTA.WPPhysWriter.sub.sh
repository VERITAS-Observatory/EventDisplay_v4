#!/bin/bash
#
# script to write CTA WP Phys Files
#
# Author: Gernot Maier
#
#######################################################################

if [ $# -ne 7 ]
then
   echo 
   echo "./CTA.WPPhysWriter.sh <sub array list> <directory with effective areas> <observation time [h]> <output file name> <offset=0/1> <recid> <data set>"
   echo
   echo "  <sub array list>          text file with list of subarray IDs"
   echo ""
   echo " <output file name>         output file name (without.root)"
   echo ""
   exit
fi

SUBAR=$1
DDIR=$2
OBSTIME=$3
OUTNAME=$4
OFFSET=$5
RECID=$6
DSET=$7

############################################################################

# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

# log files
DATE=`date +"%y%m%d"`
FDIR=$CTA_USER_LOG_DIR/$DATE/WPPHYSWRITER/
mkdir -p $FDIR
echo "log directory: " $FDIR

# script name template
FSCRIPT="CTA.WPPhysWriter.qsub"

###############################################################
# loop over all arrays
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $1`
for ARRAY in $VARRAY
do
   echo "STARTING ARRAY $ARRAY"


   ODIR=$CTA_USER_DATA_DIR/analysis/WPPhys/
   OXUTNAME=$ODIR/$OUTNAME
   mkdir -p $ODIR
   echo "WP Phys file written to $OXUTNAME"

   FNAM=$FDIR/$FSCRIPT-$ARRAY-$DSET-$OBSTIME.sh
   cp -f $FSCRIPT.sh $FNAM

   sed -i -e "s|ARRAY|$ARRAY|" \
       -e "s|DDIR|$DDIR|" \
       -e "s|OBSTIME|$OBSTIME|" \
       -e "s|OUTNAME|$OXUTNAME|" \
       -e "s|OFFSET|$OFFSET|" \
       -e "s|ODIR|$ODIR|" \
       -e "s|RRRR|$RECID|" $FNAM

   qsub -V -l os="sl*"  -l h_cpu=11:29:00 -l h_vmem=8000M -l tmpdir_size=1G -o $FDIR -e $FDIR "$FNAM"

done

exit
