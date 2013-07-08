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

   ODIR=$CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/WPPhys/
   OXUTNAME=$ODIR/$OUTNAME
   mkdir -p $ODIR
   echo "WP Phys file written to $OXUTNAME"

   rm -f $FDIR/$FSCRIPT-$ARRAY-1.sh
   cp $FSCRIPT.sh $FDIR/$FSCRIPT-$ARRAY-1.sh

   rm -f $FDIR/$FSCRIPT-$ARRAY-2.sh
   sed -e "s|ARRAY|$ARRAY|" $FDIR/$FSCRIPT-$ARRAY-1.sh > $FDIR/$FSCRIPT-$ARRAY-2.sh
   rm -f $FDIR/$FSCRIPT-$ARRAY-1.sh

   rm -f $FDIR/$FSCRIPT-$ARRAY-3.sh
   sed -e "s|DDIR|$DDIR|" $FDIR/$FSCRIPT-$ARRAY-2.sh > $FDIR/$FSCRIPT-$ARRAY-3.sh
   rm -f $FDIR/$FSCRIPT-$ARRAY-2.sh

   rm -f $FDIR/$FSCRIPT-$ARRAY-4.sh
   sed -e "s|OBSTIME|$OBSTIME|" $FDIR/$FSCRIPT-$ARRAY-3.sh > $FDIR/$FSCRIPT-$ARRAY-4.sh
   rm -f $FDIR/$FSCRIPT-$ARRAY-3.sh

   rm -f $FDIR/$FSCRIPT-$ARRAY-5.sh
   sed -e "s|OUTNAME|$OXUTNAME|" $FDIR/$FSCRIPT-$ARRAY-4.sh > $FDIR/$FSCRIPT-$ARRAY-5.sh
   rm -f $FDIR/$FSCRIPT-$ARRAY-4.sh

   rm -f $FDIR/$FSCRIPT-$ARRAY-6.sh
   sed -e "s|OFFSET|$OFFSET|" $FDIR/$FSCRIPT-$ARRAY-5.sh > $FDIR/$FSCRIPT-$ARRAY-6.sh
   rm -f $FDIR/$FSCRIPT-$ARRAY-5.sh

   rm -f $FDIR/$FSCRIPT-$ARRAY-7.sh
   sed -e "s|ODIR|$ODIR|" $FDIR/$FSCRIPT-$ARRAY-6.sh > $FDIR/$FSCRIPT-$ARRAY-7.sh
   rm -f $FDIR/$FSCRIPT-$ARRAY-6.sh

   rm -f $FDIR/$FSCRIPT-$ARRAY-8.sh
   sed -e "s|RRRR|$RECID|" $FDIR/$FSCRIPT-$ARRAY-7.sh > $FDIR/$FSCRIPT-$ARRAY-8.sh
   rm -f $FDIR/$FSCRIPT-$ARRAY-7.sh

   mv $FDIR/$FSCRIPT-$ARRAY-8.sh $FDIR/$FSCRIPT-$ARRAY.sh

   qsub -V -l os="sl*"  -l h_cpu=11:29:00 -l h_vmem=8000M -l tmpdir_size=1G -o $FDIR -e $FDIR "$FDIR/$FSCRIPT-$ARRAY.sh"

done

exit
