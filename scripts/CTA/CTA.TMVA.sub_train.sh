#!/bin/sh
#
# script to train cuts/MVAs with TMVA
#
#
# Author: Gernot Maier
#

if [ $# -ne 4 ]
then
   echo
   echo "CTA.TMVA.sub_train.sh <subarray list> <onSource/cone10> <data set> <analysis parameter file>"
   echo ""
   echo "  <subarray list>   text file with list of subarray IDs"
   echo
   echo "  <onSource/cone10>    calculate tables for on source or different wobble offsets"
   echo
   echo "  <data set>         e.g. cta-ultra3, ISDC3700, ...  "
   echo
   echo "   note 1: keywords ENERGYBINS and OUTPUTFILE are ignored in the runparameter file"
   echo
   echo "   note 2: energy and wobble offset bins are hardwired in this scripts"
   echo
   echo "   note 3: adjust h_cpu depending on your MVA method"
   echo
   echo "   note 4: default TMVA parameter file is $CTA_EVNDISP_AUX_DIR/ParameterFiles/TMVA.BDT.runparameter"
   echo
   exit
fi

#######################################
# read values from parameter file
ANAPAR=$4
if [ ! -e $ANAPAR ]
then
  echo "error: analysis parameter file not found: $ANAPAR" 
  exit
fi
echo "reading analysis parameter from $ANAPAR"
NIMAGESMIN=`grep NIMAGESMIN $ANAPAR | awk {'print $2'}`
ANADIR=`grep MSCWSUBDIRECTORY  $ANAPAR | awk {'print $2'}`
EREC=`grep ENERGYRECONSTRUCTIONMETHOD $ANAPAR | awk {'print $2'}`
DDIR=`grep TMVASUBDIR $ANAPAR | awk {'print $2'}`
RECID=`grep RECID $ANAPAR | awk {'print $2'}`
echo $NIMAGESMIN $ANADIR $EREC $DDIR
# parameters from command line
RPAR="$CTA_EVNDISP_AUX_DIR/ParameterFiles/TMVA.BDT"
RXPAR=`basename $RPAR.runparameter runparameter`
OFIL="BDT"
CONE="FALSE"
if [ $2 == "cone10" ] || [ $2 == "cone" ]
then
  CONE="TRUE"
fi
DSET=$3
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $1`

#####################################
# energy bins
# (default 2012/10/09)
# EMIN=( -2.50 -1.75 -1.25 -1.00 -0.75 -0.50 -0.25 0.00 0.25 0.50 0.75 1.00 1.50 )
# EMAX=( -1.25 -1.00 -0.75 -0.50 -0.25  0.00  0.25 0.50 0.75 1.00 1.30 1.75 2.50 )
# shortened 2012/10/09
# EMIN=( -2.50 -1.75 -1.00 -0.50 0.00 0.75 )
# EMAX=( -1.25 -0.75 -0.25  0.25 1.00 2.50 )
# shortened 2012/10/12
# EMIN=( -2.50 -1.25 -1.00 -0.50 0.00 0.75 )
# EMAX=( -1.00 -0.75 -0.25  0.25 1.00 2.50 )
# shortened 2012/10/14
# EMIN=( -2.50 -1.75 -1.25 -1.00 -0.50 0.00 0.75 )
# EMAX=( -1.25 -1.00 -0.75 -0.25  0.25 1.00 2.50 )
# extended 2013/02/11
#EMIN=( -2.50 -1.75 -1.25 -1.00 -0.50 0.00 0.75 1.35 )
#EMAX=( -1.25 -1.00 -0.75 -0.25  0.25 1.00 1.60 2.50 )
# extended 2013/03/21
#EMIN=( -2.50 -1.75 -1.25 -1.00 -0.50 0.00 0.75 1.25 )
#EMAX=( -1.25 -1.00 -0.75 -0.25  0.25 1.00 1.50 2.50 )
# removed too small HE bin (2013/05/07)
EMIN=( -2.50 -2.00 -1.50 -1.00 -0.50 0.00 0.50 1.25 )
EMAX=( -1.50 -1.25 -0.75 -0.25  0.25 0.75 1.50 2.50 )
NENE=${#EMIN[@]}
#####################################
# 
if [ $CONE == "TRUE" ]
then
   OFFMIN=( 0.0 1.0 2.0 3.00 3.50 4.00 4.50 5.00 5.50 )
   OFFMAX=( 1.0 2.0 3.0 3.50 4.00 4.50 5.00 5.50 6.00 )
   OFFMEA=( 0.5 1.5 2.5 3.25 3.75 4.25 4.75 5.25 5.75 )
   DSUF="gamma_cone10"
else
   OFFMIN=( "0.0" )
#   OFFMAX=( "1.0" )
   OFFMAX=( "1.e10" )
   OFFMEA=( 0.0 )
   DSUF="gamma_onSource"
fi
NOFF=${#OFFMIN[@]}


# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

# log files
DATE=`date +"%y%m%d"`
#QDIR=$CTA_USER_LOG_DIR/$DATE/TMVATRAINING/
#mkdir -p $QDIR
QDIR="/dev/null"
echo "log directory: " $QDIR

# script name template
FSCRIPT="CTA.TMVA.qsub_train"

###############################################################
# loop over all arrays
for ARRAY in $VARRAY
do
   echo "STARTING ARRAY $ARRAY"

# signal and background files
   if [ $CONE == "TRUE" ]
   then
      SFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/$ANADIR/$DSUF."$ARRAY"_ID"$RECID"*.mscw.root`
   else
      SFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/$ANADIR/$DSUF."$ARRAY"_ID"$RECID"*.mscw.root`
   fi
   BFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/$ANADIR/proton."$ARRAY"_ID"$RECID"*.root`

   for (( W = 0; W < $NOFF; W++ ))
   do
      ODIR=$CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/TMVA/$DDIR-${OFFMEA[$W]}
      mkdir -p $ODIR
# copy run parameter file
      cp -f $RPAR.runparameter $ODIR

###############################################################
# loop over all energy bins and submit a job for each bin
      for ((i=0; i < $NENE; i++))
      do

# updating the  run parameter file
	 RFIL=$ODIR/$RXPAR$ARRAY"_$i"
	 echo $RFIL
	 rm -f $RFIL
	 echo "* ENERGYBINS $EREC ${EMIN[$i]} ${EMAX[$i]}" > $RFIL.runparameter
	 echo "* MCXYOFF (MCxoff*MCxoff+MCyoff*MCyoff)>=${OFFMIN[$W]}*${OFFMIN[$W]}&&(MCxoff*MCxoff+MCyoff*MCyoff)<${OFFMAX[$W]}*${OFFMAX[$W]}" >> $RFIL.runparameter
	 grep "*" $RPAR.runparameter | grep -v ENERGYBINS | grep -v OUTPUTFILE | grep -v SIGNALFILE | grep -v BACKGROUNDFILE | grep -v MCXYOFF >> $RFIL.runparameter
	 echo "* OUTPUTFILE $ODIR $OFIL"_$i" " >> $RFIL.runparameter
	 for arg in $SFIL
	 do
	    echo "* SIGNALFILE $arg" >> $RFIL.runparameter
	 done
	 for arg in $BFIL
	 do
	    echo "* BACKGROUNDFILE $arg" >> $RFIL.runparameter
	 done
# setting the cuts correctly in the run parameter file
         sed -i "s|MINIMAGES|$NIMAGESMIN|" $RFIL.runparameter
	 if [ $EREC = "0" ]
	 then
	    sed -i 's|ENERGYVARIABLE|Erec|' $RFIL.runparameter
	    sed -i 's|ENERGYCHI2VARIABLE|EChi2|g' $RFIL.runparameter
         else
	    sed -i 's|ENERGYVARIABLE|ErecS|' $RFIL.runparameter
	    sed -i 's|ENERGYCHI2VARIABLE|EChi2S|g' $RFIL.runparameter
         fi

# run script
	 FNAM=$ODIR/$FSCRIPT.$ARRAY"_$i"

	 sed -e "s|RUNPARA|$RFIL|" $FSCRIPT.sh > $FNAM-1.sh
	 sed -e "s|OFIL|$ODIR/$OFIL"_$i"|" $FNAM-1.sh > $FNAM.sh
	 rm -f $FNAM-1.sh

	 chmod u+x $FNAM.sh
	 echo $FNAM.sh

#################################
# submit job to queue
# medium queue: BDT 
	 qsub -js 200 -V -l os="sl*" -l h_cpu=11:29:00 -l h_vmem=8000M -l tmpdir_size=5G -o $QDIR -e $QDIR "$FNAM.sh"
# long queue: needed for box cut optimization
#         qsub -V -l h_cpu=38:00:00 -l h_vmem=8000M -l tmpdir_size=5G -o $QDIR -e $QDIR "$FNAM.sh"
     done
  done
done

exit
