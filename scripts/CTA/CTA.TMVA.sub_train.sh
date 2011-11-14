#!/bin/sh
#
# script to train cuts/MVAs with TMVA
#
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ]
then
   echo
   echo "CTA.TMVA.sub_train.sh <array ID> <run parameter filename> <directory for run parameter and log files> <output file name>"
   echo ""
   echo "  <array ID>         CTA array ID (e.g. E for array E)"
   echo "                     use ALL for all arrays (A B C D E F G H I J K NA NB)"
   echo "<run parameter filename> without .runparameter"
   echo " should be located in the output directory"
   echo
   echo "   note 1: keywords ENERGYBINS and OUTPUTFILE are ignored in the runparameter file"
   echo
   echo "   note 2: energy bins are hardwired in this scripts"
   echo
   echo "   note 3: adjust h_cpu depending on your MVA method"
   echo
   exit
fi

ARRAY=$1
RPAR=$2
RXPAR=`basename $RPAR.runparameter runparameter`
DDIR=$3
OFIL=$4
# energy reconstruction method
EREC=0

if [ $ARRAY == "ALL" ]
then
  VARRAY=( A B C D E F G H I J K NA NB "s4-1-120" "s4-2-120" "s4-2-85" )
else 
  VARRAY=( $ARRAY )
fi
NARRAY=${#VARRAY[@]}

#####################################
# energy bins
EMIN=( -2.50 -1.75 -1.25 -1.00 -0.75 -0.50 -0.25 0.00 0.25 0.50 0.75 1.00 )
EMAX=( -1.25 -1.00 -0.75 -0.50 -0.25  0.00  0.25 0.50 0.75 1.00 1.50 2.50 )
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

# script name template
FSCRIPT="CTA.TMVA.qsub_train"

###############################################################
# loop over all arrays
for (( N = 0 ; N < $NARRAY; N++ ))
do
   ARRAY=${VARRAY[$N]}
   echo "STARTING ARRAY $ARRAY"

   ODIR=$CTA_USER_DATA_DIR/analysis/$ARRAY/TMVA/$DDIR
   mkdir -p $ODIR
# copy run parameter file
   cp -f $RPAR.runparameter $ODIR

# signal and background files
   SFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/$ARRAY/Analysis/gamma_onSource."$ARRAY"_ID0*.root`
   BFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/$ARRAY/Analysis/proton."$ARRAY"_ID0*.root`

###############################################################
# loop over all energy bins and submit a job for each bin
   for ((i=0; i < $NENE; i++))
   do

# run parameter file
      RFIL=$ODIR/$RXPAR$ARRAY"_$i"
      echo $RFIL
      rm -f $RFIL
      echo "* ENERGYBINS $EREC ${EMIN[$i]} ${EMAX[$i]}" > $RFIL.runparameter
      grep "*" $RPAR.runparameter | grep -v ENERGYBINS | grep -v OUTPUTFILE | grep -v SIGNALFILE | grep -v BACKGROUNDFILE >> $RFIL.runparameter
      echo "* OUTPUTFILE $ODIR $OFIL"_$i" " >> $RFIL.runparameter
      for arg in $SFIL
      do
	 echo "* SIGNALFILE $arg" >> $RFIL.runparameter
      done
      for arg in $BFIL
      do
	 echo "* BACKGROUNDFILE $arg" >> $RFIL.runparameter
      done

# run script
      FNAM=$ODIR/$FSCRIPT.$ARRAY"_$i"

      sed -e "s|RUNPARA|$RFIL|" $FSCRIPT.sh > $FNAM-1.sh
      sed -e "s|OFIL|$ODIR/$OFIL"_$i"|" $FNAM-1.sh > $FNAM.sh
      rm -f $FNAM-1.sh

      chmod u+x $FNAM.sh
      echo $FNAM.sh

#################################
# submit job to queue
# short queue: BDT 
      qsub -V -l h_cpu=10:29:00 -l h_vmem=8000M -l tmpdir_size=5G -o $FDIR -e $FDIR "$FNAM.sh"
# long queue: needed for box cut optimization
#      qsub -V -l h_cpu=38:00:00 -l h_vmem=8000M -l tmpdir_size=5G -o $FDIR -e $FDIR "$FNAM.sh"
  done
done

exit
