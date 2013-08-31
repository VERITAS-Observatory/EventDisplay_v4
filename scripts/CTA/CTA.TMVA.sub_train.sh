#!/bin/sh
#
# script to train cuts/MVAs with TMVA
#
#
# Author: Gernot Maier
#

if [ $# -ne 4 ] && [ $# -ne 5 ]
then
   echo
   echo "CTA.TMVA.sub_train.sh <subarray list> <onSource/cone> <data set> <analysis parameter file> [direction (e.g. _180deg)]"
   echo ""
   echo "  <subarray list>   text file with list of subarray IDs"
   echo
   echo "  <onSource/cone>    calculate tables for on source or different wobble offsets"
   echo
   echo "  <data set>         e.g. cta-ultra3, ISDC3700, ...  "
   echo 
   echo "  <direction>        e.g. for north: \"_180deg\", for south: \"_0deg\", for all directions: no option"
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
if [[ $2 == cone ]]
then
  CONE="TRUE"
fi
DSET=$3
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $1`

#####################################
if [ -n "$5" ]
then
  MCAZ=$5
fi

#####################################
# energy bins
EMIN=( -1.90 -1.90 -1.45 -1.00 -0.50 0.00 0.50 1.25 )
EMAX=( -1.40 -1.30 -0.75 -0.25  0.25 0.75 1.50 2.50 )
NENE=${#EMIN[@]}
#####################################
# offset bins 
if [ $CONE == "TRUE" ]
then
   OFFMIN=( 0.0 1.0 2.0 3.00 3.50 4.00 4.50 5.00 5.50 )
   OFFMAX=( 1.0 2.0 3.0 3.50 4.00 4.50 5.00 5.50 6.00 )
   OFFMEA=( 0.5 1.5 2.5 3.25 3.75 4.25 4.75 5.25 5.75 )
   DSUF="gamma_cone"
else
   OFFMIN=( "0.0" )
   OFFMAX=( "1.e10" )
   OFFMEA=( 0.0 )
   DSUF="gamma_onSource"
fi
NOFF=${#OFFMIN[@]}

######################################
# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

######################################
# log files
DATE=`date +"%y%m%d"`
LDIR=$CTA_USER_LOG_DIR/$DATE/TMVATRAINING/
mkdir -p $LDIR
QDIR=$LDIR
echo "log directory: " $LDIR
echo "queue log directory: " $QDIR

######################################
# script name template
FSCRIPT="CTA.TMVA.qsub_train"

###############################################################
# loop over all arrays
for ARRAY in $VARRAY
do
   echo "STARTING ARRAY $ARRAY"

# signal and background files (depending on on-axis or cone data set)
   if [ $CONE == "TRUE" ]
   then
      SFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/$ANADIR/$DSUF."$ARRAY"_ID"$RECID$MCAZ"*.mscw.root`
      BFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/$ANADIR/proton."$ARRAY"_ID"$RECID$MCAZ"*.root`
   else
      SFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/$ANADIR"-onAxis"/$DSUF."$ARRAY"_ID"$RECID$MCAZ"*.mscw.root`
      BFIL=`ls -1 $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/$ANADIR"-onAxis"/proton."$ARRAY"_ID"$RECID$MCAZ"*.root`
   fi


###############################################################
# loop over all wobble offset
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
# setting the chosen energy variable
	 if [ $EREC = "0" ]
	 then
	    sed -i 's|ENERGYVARIABLE|Erec|' $RFIL.runparameter
	    sed -i 's|ENERGYCHI2VARIABLE|EChi2|g' $RFIL.runparameter
	    sed -i 's|ENERGYDEVARIABLE|dE|g' $RFIL.runparameter
         else
	    sed -i 's|ENERGYVARIABLE|ErecS|' $RFIL.runparameter
	    sed -i 's|ENERGYCHI2VARIABLE|EChi2S|g' $RFIL.runparameter
	    sed -i 's|ENERGYDEVARIABLE|dES|g' $RFIL.runparameter
         fi

# run script
	 FNAM=$LDIR/$FSCRIPT.$DSET.$ARRAY.$2."_$i"

	 sed -e "s|RUNPARA|$RFIL|" \
	     -e "s|OFIL|$ODIR/$OFIL"_$i"|" $FSCRIPT.sh  > $FNAM.sh
	 rm -f $FNAM-1.sh

	 chmod u+x $FNAM.sh
	 echo $FNAM.sh

#################################
# submit job to queue
	 qsub -V -l os="sl*" -l h_cpu=41:29:00 -l h_vmem=8000M -l tmpdir_size=5G -o $QDIR -e $QDIR "$FNAM.sh"
     done
  done
done

exit
