#!/bin/bash
#
# script to write particle rate files for CTA
#
# Author: Gernot Maier
#
#######################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ] 
then
   echo 
   echo "./CTA.ParticleRateWriter.sub.sh <sub array list> <directory with effective areas> <offset=onSource/cone10> <recid> "
   echo 
   echo "  write particles files needed for TMVA cut optimization"
   echo
   echo "  <sub array list>          text file with list of subarray IDs"
   echo
   echo "  <directory with effective areas>  (full) path to effective areas"
   echo
   echo "  <recid>                   reconstruction ID from mscw stage" 
   echo ""
   exit
fi

SUBAR=$1
DDIR=$2
OFFSET=$3
RECID=$4

############################################################################

# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

# log files
DATE=`date +"%y%m%d"`
FDIR=$CTA_USER_LOG_DIR/queueParticleRateWriter/$DATE
mkdir -p $FDIR
echo "log directory: " $FDIR

# script name template
FSCRIPT="CTA.ParticleRateWriter.qsub"

ODIR=$DDIR

###############################################################
# loop over all arrays
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $1`
for ARRAY in $VARRAY
do
   echo "STARTING ARRAY $ARRAY"

   rm -f $ODIR/$FSCRIPT-$ARRAY-1.sh
   cp $FSCRIPT.sh $ODIR/$FSCRIPT-$ARRAY-1.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-2.sh
   sed -e "s|ARRAY|$ARRAY|" $ODIR/$FSCRIPT-$ARRAY-1.sh > $ODIR/$FSCRIPT-$ARRAY-2.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-1.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-3.sh
   sed -e "s|DDIR|$DDIR|" $ODIR/$FSCRIPT-$ARRAY-2.sh > $ODIR/$FSCRIPT-$ARRAY-3.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-2.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-4.sh
   sed -e "s|RRRR|$RECID|" $ODIR/$FSCRIPT-$ARRAY-3.sh > $ODIR/$FSCRIPT-$ARRAY-4.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-3.sh

   rm -f $ODIR/$FSCRIPT-$ARRAY-5.sh
   sed -e "s|OFFSET|$OFFSET|" $ODIR/$FSCRIPT-$ARRAY-4.sh > $ODIR/$FSCRIPT-$ARRAY-5.sh
   rm -f $ODIR/$FSCRIPT-$ARRAY-4.sh

   mv $ODIR/$FSCRIPT-$ARRAY-5.sh $ODIR/$FSCRIPT-$ARRAY.sh

   qsub -V -js 20 -l os="sl*"  -l h_cpu=0:29:00 -l h_vmem=4000M -l tmpdir_size=1G -o $ODIR -e $ODIR "$ODIR/$FSCRIPT-$ARRAY.sh"

done

exit
