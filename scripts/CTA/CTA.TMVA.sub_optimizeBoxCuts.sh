#!/bin/sh
#
# script to optimize cuts with TMVA
#
# Revision $Id: mm_optimizeBoxCuts_TMVA.sh,v 1.1.2.1 2011/04/11 16:11:21 gmaier Exp $
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ]
then
   echo "CTA.TMVA.sub_optimizeBoxCuts.sh <run parameter filename> <directory for run parameter and log files> <output file name> [erec method]"
   echo ""
   echo "<run parameter filename> without .runparameter"
   echo " should be located in the output directory"
   echo
   echo "[erec method]  energy reconstruction method (default=0)"
   echo
   echo "note 1: keywords ENERGYBINS and OUTPUTFILE are ignored in the runparameter file"
   echo
   echo "note 2: energy bins are hardwired in this scripts"
   echo
   exit
fi

RPAR=$1
ODIR=$2
mkdir -p $ODIR
OFIL=$3
# energy reconstruction method
EREC=0
if [ -n "$4" ]
then
  EREC="$4"
fi

#####################################
# energy bins
#EMIN=( -2.5 -1.0 -0.5 0.0 0.5 1.0 )
#EMAX=( -1.0 -0.5  0.0 0.5 1.0 2.5 )
EMIN=( -2.50 -1.75 -1.25 -1.00 -0.75 -0.50 -0.25 0.00 0.25 0.50 0.75 1.00 )
EMAX=( -1.25 -1.00 -0.75 -0.50 -0.25  0.00  0.25 0.50 0.75 1.00 1.50 2.50 )
#EMIN=( -2.50 -1.00 -0.75 -0.50 -0.25 0.00 0.25 0.50 )
#EMAX=( -0.50 -0.25  0.00  0.25  0.50 0.75 1.00 2.50 )
NENE=${#EMIN[@]}
#####################################


# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

# log files
DATE=`date +"%y%m%d"`
FDIR=$CTA_USER_LOG_DIR/queueTMVA/$DATE
mkdir -p $FDIR
echo "log directory: " $FDIR

# copy run parameter file
cp -f $RPAR.runparameter $ODIR

# script name template
FSCRIPT="CTA.TMVA.qsub_optimizeBoxCuts"

###############################################################
# loop over all energy bins and submit a job for each bin
for ((i=0; i < $NENE; i++))
do

# run parameter file
   echo $RFIL
   RFIL=$ODIR/$RPAR"_$i"
   rm -f $RFIL
   echo "* ENERGYBINS $EREC ${EMIN[$i]} ${EMAX[$i]}" > $RFIL.runparameter
   echo "* OUTPUTFILE $ODIR $OFIL"_$i" " >> $RFIL.runparameter
   grep "*" $RPAR.runparameter | grep -v ENERGYBINS | grep -v OUTPUTFILE >> $RFIL.runparameter

# run script
  FNAM=$ODIR/$FSCRIPT"_$i"

  sed -e "s|RUNPARA|$RFIL|" $FSCRIPT.sh > $FNAM-1.sh
  sed -e "s|OFIL|$ODIR/$OFIL"_$i"|" $FNAM-1.sh > $FNAM.sh
  rm -f $FNAM-1.sh

  chmod u+x $FNAM.sh
  echo $FNAM.sh

# submit job to queue

  qsub -V -l h_cpu=38:00:00 -l h_vmem=8000M -l tmpdir_size=5G -o $FDIR -e $FDIR "$FNAM.sh"

done

exit
