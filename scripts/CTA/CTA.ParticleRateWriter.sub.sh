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
FDIR=$CTA_USER_LOG_DIR/$DATE/PARTICLERATEWRITER/
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

   rm -f $ODIR/$FSCRIPT-$ARRAY.sh
   cp $FSCRIPT.sh $FDIR/$FSCRIPT-$ARRAY.sh

   sed -i -e "s|ARRAY|$ARRAY|" \
          -e "s|DDIR|$DDIR|" \
          -e "s|RRRR|$RECID|" \
          -e "s|OFFSET|$OFFSET|" $FDIR/$FSCRIPT-$ARRAY.sh

   qsub -js 2000 -P cta_high -V -l os="sl*"  -l h_cpu=11:29:00 -l h_vmem=4000M -l tmpdir_size=1G -o $FDIR -e $FDIR "$FDIR/$FSCRIPT-$ARRAY.sh"

done

exit
