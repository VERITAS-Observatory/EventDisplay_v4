#!/bin/bash
# script to train BDTs with TMVA

# qsub parameters
h_cpu=11:59:59; h_vmem=11599M; tmpdir_size=24G

if [[ $# < 10 ]]; then
# begin help message
echo "
TMVA training of BDT: submit jobs from a TMVA runparameter file

ANALYSIS.tmva_bdt.sh <list of background files> <TMVA runparameter file> <output directory> <output file name> <sim type>
 [epoch] [atmosphere] [Rec ID]

required parameters:

    <list of background files>      list of background training samples with whole path to each file
    
    <TMVA runparameter file>        TMVA runparameter file with basic options (incl. whole range of 
	                                energy and zenith angle bins) and full path
    
    <output directory>              BDT files are written to this directory
    
    <output file name>              name of output file e.g. BDT  

    <sim type>                      original VBF file simulation type (e.g. GRISU, CARE)

optional parameters:

    [epoch]                         array epoch e.g. V4, V5, V6
                                    default: \"V6\"

    [atmosphere]                    atmosphere model(s) (21 = winter, 22 = summer)
                                    default: \"21 22\"                   

    [Rec ID]                        reconstruction ID(s) (default: \"0\")
                                    (see EVNDISP.reconstruction.runparameter)	    

additional info:

    energy and zenith angle bins should be indicated in the runparameter file with basic options
--------------------------------------------------------------------------------
"
#end help message
exit
fi
echo " "
# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
BLIST=$1
RUNPAR=$2
ODIR=$3
ONAME=$4
SIMTYPE=$5
if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
    SIMTYPE="GRISU-SW6"
elif [[ ${SIMTYPE:0:4} = "CARE" ]]; then 
    SIMTYPE="CARE_June1425"
else
    echo "Invalid simulation type. Exiting..."
    exit 1
fi
[[ "$6" ]] && EPOCH=$6 || EPOCH="V6"
[[ "$7" ]] && ATM=$7 || ATM="21 22"
[[ "$8" ]] && RECID=$8 || RECID="0"

PARTICLE_TYPE="gamma"
# evndisplay version
IRFVERSION=`$EVNDISPSYS/bin/mscw_energy --version | tr -d .| sed -e 's/[a-Z]*$//'`

# Check that list of background files exists
if [[ ! -f "$BLIST" ]]; then
    echo "Error, list of background files $BLIST not found, exiting..."
    exit 1
fi

# Check that TMVA run parameter file exists
if [[ "$RUNPAR" == `basename $RUNPAR` ]]; then
    RUNPAR="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/$RUNPAR"
fi
if [[ ! -f "$RUNPAR" ]]; then
    echo "Error, TMVA run parameter file $RUNPAR not found, exiting..."
    exit 1
fi

RXPAR=`basename $RUNPAR .runparameter`
echo "Original TMVA run parameter file: $RXPAR.runparameter "

# output directory
echo -e "Output files will be written to:\n $ODIR/RecID${RECID}"
mkdir -p $ODIR/RecID${RECID}

#####################################
# energy bins
ENBINS=$( cat "$RUNPAR" | grep "^* ENERGYBINS 1" | sed -e 's/* ENERGYBINS 1//' | sed -e 's/ /\n/g')
declare -a EBINARRAY=( $ENBINS ) #convert to array
count1=1
NENE=$((${#EBINARRAY[@]}-$count1)) #get number of bins

####################################
# energy reconstruction method
echo "Energy reconstruction method: 1 (hard-coded)"
#####################################
# zenith angle bins
ZEBINS=$( cat "$RUNPAR" | grep "^* ZENBINS " | sed -e 's/* ZENBINS//' | sed -e 's/ /\n/g')

declare -a ZEBINARRAY=( $ZEBINS ) #convert to array
NZEW=$((${#ZEBINARRAY[@]}-$count1)) #get number of bins

#####################################
# zenith angle bins of MC simulation files
# ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
ZENITH_ANGLES=( 20 30 35 40 45 50 55 )

#####################################
# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/TMVA.ANADATA"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.trainTMVAforGammaHadronSeparation_sub"

###############################################################
# loop over all energy bins and submit a job for each bin
for (( i=0; i < $NENE; i++ ))
do
   echo "==========================================================================="
   echo " "
   echo "EBin: $(($i+$count1)) of $NENE: ${EBINARRAY[$i]} to ${EBINARRAY[$i+1]}"
##############################################
# loop over all zenith angle bins
   for (( j=0; j < $NZEW; j++ ))
   do
      echo "---------------------------------------------------------------------------"
      echo "ZeBin: $(($j+$count1)) of $NZEW: ${ZEBINARRAY[$j]} to ${ZEBINARRAY[$j+1]}"
      
      # copy run parameter file with basic options to output directory
      cp -f $RUNPAR $ODIR

      # updating the run parameter file for each parameter space
      RFIL=$ODIR/RecID${RECID}/$RXPAR"_$i""_$j"
      echo "TMVA Runparameter file: $RFIL.runparameter"
      rm -f $RFIL
      
      echo "* ENERGYBINS 1 ${EBINARRAY[$i]} ${EBINARRAY[$i+1]}" > $RFIL.runparameter
      echo "* ZENBINS  ${ZEBINARRAY[$j]} ${ZEBINARRAY[$j+1]}" >> $RFIL.runparameter
      grep "*" $RUNPAR | grep -v ENERGYBINS | grep -v ZENBINS | grep -v OUTPUTFILE | grep -v SIGNALFILE | grep -v BACKGROUNDFILE | grep -v MCXYOFF >> $RFIL.runparameter
    
      nTrainSignal=200000
      if  [ "$i" -eq "0" ] && [ "$j" -eq "0" ]; then
          nTrainBackground=200000
      elif  [ "$i" -eq "0" ] && [ "$j" -eq "1" ]; then
          nTrainBackground=200000
      elif  [ "$i" -eq "0" ] && [ "$j" -eq "2" ]; then
          nTrainBackground=200000
      elif  [ "$i" -eq "0" ] && [ "$j" -eq "3" ]; then
          nTrainSignal=50000
          nTrainBackground=0
      elif  [ "$i" -eq "1" ] && [ "$j" -eq "0" ]; then
          nTrainSignal=200000
          nTrainBackground=200000
      elif  [ "$i" -eq "1" ] && [ "$j" -eq "1" ]; then
          nTrainSignal=200000
      elif  [ "$i" -eq "1" ] && [ "$j" -eq "2" ]; then
          nTrainBackground=200000
      elif  [ "$i" -eq "1" ] && [ "$j" -eq "3" ]; then
          nTrainBackground=200000
      elif  [ "$i" -eq "2" ] && [ "$j" -eq "0" ]; then
          nTrainSignal=100000
          nTrainBackground=100000
      elif  [ "$i" -eq "2" ] && [ "$j" -eq "1" ]; then
          nTrainSignal=100000
          nTrainBackground=0
      elif  [ "$i" -eq "2" ] && [ "$j" -eq "2" ]; then
          nTrainSignal=100000
          nTrainBackground=100000
      elif  [ "$i" -eq "2" ] && [ "$j" -eq "3" ]; then
          nTrainSignal=200000
      elif  [ "$i" -eq "3" ] && [ "$j" -eq "0" ]; then
          nTrainSignal=50000
          nTrainBackground=0
      elif  [ "$i" -eq "3" ] && [ "$j" -eq "1" ]; then
          nTrainSignal=28000
          nTrainBackground=0
      elif  [ "$i" -eq "3" ] && [ "$j" -eq "2" ]; then
          nTrainSignal=50000
          nTrainBackground=0
      elif  [ "$i" -eq "3" ] && [ "$j" -eq "3" ]; then
          nTrainSignal=100000
          nTrainBackground=0
      fi
      echo "* PREPARE_TRAINING_OPTIONS SplitMode=Random:!V:nTrain_Signal=$nTrainSignal:nTrain_Background=$nTrainBackground::nTest_Signal=$nTrainSignal:nTest_Background=$nTrainBackground" >> $RFIL.runparameter

      echo "* OUTPUTFILE $ODIR/RecID${RECID} $ONAME"_$i""_$j" " >> $RFIL.runparameter

      echo "#######################################################################################" >> $RFIL.runparameter
      # signal and background files (depending on on-axis or cone data set)
      for ATMX in $ATM; do
          SDIR="$VERITAS_IRFPRODUCTION_DIR/$IRFVERSION/$SIMTYPE/${EPOCH}_ATM${ATMX}_${PARTICLE_TYPE}/MSCW_RECID${RECID}"
          echo "Signal input directory: $SDIR"
          if [[ ! -d $SDIR ]]; then
              echo -e "Error, could not locate directory of simulation files (input). Locations searched:\n $SDIR"
              exit 1
          fi
          if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
              for (( l=0; l < ${#ZENITH_ANGLES[@]}; l++ ))
              do
                  if (( $(echo "${ZEBINARRAY[$j]} <= ${ZENITH_ANGLES[$l]}" | bc ) && $(echo "${ZEBINARRAY[$j+1]} >= ${ZENITH_ANGLES[$l]}" | bc ) ));then
                      if (( "${ZENITH_ANGLES[$l]}" != "00" && "${ZENITH_ANGLES[$l]}" != "60" && "${ZENITH_ANGLES[$l]}" != "65" )); then 
                          SIGNALLIST=`ls -1 $SDIR/${ZENITH_ANGLES[$l]}deg_0.5wob_NOISE{100,150,200,250,325,425,550}.mscw.root`
                          for arg in $SIGNALLIST
                          do
                              echo "* SIGNALFILE $arg" >> $RFIL.runparameter
                          done
                      fi
                  fi
              done
          else
              for (( l=0; l < ${#ZENITH_ANGLES[@]}; l++ ))
              do
                  if (( $(echo "${ZEBINARRAY[$j]} <= ${ZENITH_ANGLES[$l]}" | bc ) && $(echo "${ZEBINARRAY[$j+1]} >= ${ZENITH_ANGLES[$l]}" | bc ) ));then
                      if (( "${ZENITH_ANGLES[$l]}" != "00" && "${ZENITH_ANGLES[$l]}" != "60" && "${ZENITH_ANGLES[$l]}" != "65" )); then
                          SIGNALLIST=`ls -1 $SDIR/${ZENITH_ANGLES[$l]}deg_0.5wob_NOISE{50,80,120,170,230}.mscw.root`
                          for arg in $SIGNALLIST
                          do
                              echo "* SIGNALFILE $arg" >> $RFIL.runparameter
                          done
                      fi
                  fi
              done
          fi
      done 
      echo "#######################################################################################" >> $RFIL.runparameter
   	for arg in $(cat $BLIST)
   	do
         echo "* BACKGROUNDFILE $arg" >> $RFIL.runparameter
   	done
         
      FSCRIPT=$LOGDIR/TMVA.$ONAME"_$i""_$j"
      sed -e "s|RUNPARAM|$RFIL|"  \
          -e "s|OUTNAME|$ODIR/RecID${RECID}/$ONAME_${i}_${j}|" $SUBSCRIPT.sh > $FSCRIPT.sh

      chmod u+x $FSCRIPT.sh
      echo $FSCRIPT.sh

      # run locally or on cluster
      SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
      SUBC=`eval "echo \"$SUBC\""`
      if [[ $SUBC == *qsub* ]]; then
         JOBID=`$SUBC $FSCRIPT.sh`
         # account for -terse changing the job number format
         if [[ $SUBC != *-terse* ]] ; then
            echo "without -terse!"      # need to match VVVVVVVV  8539483  and 3843483.1-4:2
            JOBID=$( echo "$JOBID" | grep -oP "Your job [0-9.-:]+" | awk '{ print $3 }' )
         fi
    
         echo "JOBID:  $JOBID"
    
      elif [[ $SUBC == *parallel* ]]; then
         echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
         cat $LOGDIR/runscripts.dat | $SUBC
      fi
   done
done

exit
