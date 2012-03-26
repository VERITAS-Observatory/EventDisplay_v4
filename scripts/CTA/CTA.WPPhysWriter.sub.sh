#!/bin/bash
#
# script to write WP Phys Files
#
# Revision $Id$
#
# Author: Gernot Maier
#
#######################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ] && [ ! -n "$5" ]
then
   echo "./CTA.WPPhysWriter.sh <sub array list> <directory with effective areas> <observation time [h]> <output file name> <offset=0/1> <data set>"
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
DSET=$6

############################################################################

# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

# log files
DATE=`date +"%y%m%d"`
FDIR=$CTA_USER_LOG_DIR/queueWPPhysWriter/$DATE
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

   rm -f $ODIR/$FSCRIPT-$ARRAY-1.sh
   cp $FSCRIPT.sh $ODIR/$FSCRIPT-$ARRAY-1.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-2.sh
   sed -e "s|ARRAY|$ARRAY|" $ODIR/$FSCRIPT-$ARRAY-1.sh > $ODIR/$FSCRIPT-$ARRAY-2.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-1.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-3.sh
   sed -e "s|DDIR|$DDIR|" $ODIR/$FSCRIPT-$ARRAY-2.sh > $ODIR/$FSCRIPT-$ARRAY-3.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-2.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-4.sh
   sed -e "s|OBSTIME|$OBSTIME|" $ODIR/$FSCRIPT-$ARRAY-3.sh > $ODIR/$FSCRIPT-$ARRAY-4.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-3.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-5.sh
   sed -e "s|OUTNAME|$OXUTNAME|" $ODIR/$FSCRIPT-$ARRAY-4.sh > $ODIR/$FSCRIPT-$ARRAY-5.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-4.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-6.sh
   sed -e "s|OFFSET|$OFFSET|" $ODIR/$FSCRIPT-$ARRAY-5.sh > $ODIR/$FSCRIPT-$ARRAY-6.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-5.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-7.sh
   sed -e "s|ODIR|$ODIR|" $ODIR/$FSCRIPT-$ARRAY-6.sh > $ODIR/$FSCRIPT-$ARRAY-7.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-6.sh

   mv $ODIR/$FSCRIPT-$ARRAY-7.sh $ODIR/$FSCRIPT-$ARRAY.sh

   qsub -V -l os="sl*"  -l h_cpu=0:29:00 -l h_vmem=4000M -l tmpdir_size=1G -o $ODIR -e $ODIR "$ODIR/$FSCRIPT-$ARRAY.sh"

done

exit
