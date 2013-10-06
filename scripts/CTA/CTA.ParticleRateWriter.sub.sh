#!/bin/bash
#
# script to write particle rate files for CTA
#
# Author: Gernot Maier
#
#######################################################################

if [ $# -ne 4 ] && [ $# -ne 5 ]
then
   echo 
   echo "./CTA.ParticleRateWriter.sub.sh <sub array list> <directory with effective areas> <offset=onSource/cone> <recid> [directory with angular resolution files]"
   echo 
   echo "  write particles files needed for TMVA cut optimization"
   echo
   echo "  <sub array list>          text file with list of subarray IDs"
   echo
   echo "  <directory with effective areas>  (full) path to effective areas"
   echo
   echo "  <recid>                   reconstruction ID from mscw stage" 
   echo 
   echo "  <directory with angular resolution files> (full) path to angular resolution files"
   echo ""
   exit
fi

SUBAR=$1
DDIR=$2
OFFSET=$3
RECID=$4
ADIR=""
if [ -n "$5" ]
then
   ADIR=$5
fi

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
          -e "s|ADIR|$ADIR|" \
          -e "s|OFFSET|$OFFSET|" $FDIR/$FSCRIPT-$ARRAY.sh

   qsub -V -l os="sl*"  -l h_cpu=11:29:00 -l h_vmem=4000M -l tmpdir_size=1G -o $FDIR -e $FDIR "$FDIR/$FSCRIPT-$ARRAY.sh"
done

exit
