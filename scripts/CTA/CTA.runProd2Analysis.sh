#!/bin/sh
#
# analysis submission for production 2 analysis
#
# this script is optimized for the DESY analysis
#
##############################################


if [ $# -ne 2 ]
then
   echo 
   echo "./CTA.runProd2Analysis.sh <N/S> <run mode>"
   echo
   echo "  N=prod2-North, S=prod2-South"
   echo
   echo "  possible run modes are EVNDISP MAKETABLES ANATABLES TRAIN ANGRES QC PARTFIL CUTS PHYS "
   echo
   exit
fi
P2="$1"
RUN="$2"

#####################################
# qsub options
QSUBOPT="-P cta_high -js 2000"
QSUBOPT="-P cta_high"

#####################################
# output directory for script parameter files
PDIR="$CTA_USER_LOG_DIR/tempRunParameterDir/"
mkdir -p $PDIR

#####################################
# sites & sub array lists
if [[ $P2 == "S" ]]
then
   SITE=( "prod2-LeoncitoPP-NS" "prod2-Aar-NS" "prod2-SAC100-NS" "prod2-SAC084-NS" "prod2-Leoncito-lowE-NS" "prod2-Aar-lowE-NS" "prod2-SAC100-lowE-NS" "prod2-SAC084-lowE-NS" "prod2-Leoncito-NS" "prod2-LeoncitoTrigv2-NS" "prod2-Aar-500m-NS" )
   SITE=( "prod2-Aar-500m-NS" )
   ARRAY="subArray.2a.list"
elif [[ $P2 == "N" ]]
then
   SITE=( "prod2-US-NS" "prod2-SPM-NS" "prod2-Tenerife-NS" )
   ARRAY="subArray.2NN.list"
   ARRAY="subArray.2NN-fullList.list"
   ARRAY="subArray.2NN-sub.list"
else
   echo "error: unknown site; allowed are N or S"
   exit
fi

#####################################
# particle types
PARTICLE=( "gamma_onSource" "gamma_cone" "electron" "proton" )
PARTICLE=( "gamma_onSource" )

#####################################
# shower directions
#
# _180deg = south
# _0deg = north
MCAZ=( "_180deg" "_0deg" "" )

#####################################
# reconstruction IDs
RECID="0"

#####################################
# energy reconstruction
# (default is method 1)
EREC="1"

#####################################
# cut on number of images
NIMAGESMIN="2"

#####################################
# observing time [h]
OBSTIME=( "5h" "30m" "10m" "1m" "20s" )
OBSTIME=( "50h" "5h" "30m" "10m" "1m" "20s" )
OBSTIME=( "5h" "50h" )
OBSTIME=( "30m" )


#####################################
# analysis dates and table dates
DATE="d20130830"
TDATE="d20130812"

#####################################
# loop over all sites
NSITE=${#SITE[@]}
for (( m = 0; m < $NSITE ; m++ ))
do
   S=${SITE[$m]}

   echo
   echo "======================================================================================"
   echo "SITE: $S"
   echo "RUN: $RUN"

#####################################
# trigmask files
# (needed only for prod2 files with trigger bug)
   if [[ $S == "prod2-Aar-NS" ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/Aar/simtel/trgmask/"
   elif [[ $S == "prod2-Leoncito-NS" ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/Leoncito/simtel/trgmask/"
   elif [[ $S == "prod2-SAC084-NS" ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/SAC084/"
   elif [[ $S == "prod2-SAC100-NS" ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/SAC100/"
   else
      TRG=""
   fi

##########################################
# run eventdisplay
    if [[ $RUN == "EVNDISP" ]]
    then

# loop over all particle types
      for ((i = 0; i < ${#PARTICLE[@]}; i++ ))
      do
	  N=${PARTICLE[$i]}
	  LIST=/afs/ifh.de/group/cta/scratch/maierg/LOGS/CTA/runLists/prod2/$S.$N"_20deg".list

          ./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob.prod2.sh $ARRAY $LIST $N $S 0 $i $TRG $QSUBOPT
       done
       continue
    fi
##########################################
# loop over all reconstruction IDs
    for ID in $RECID
    do
       MSCWSUBDIRECTORY="Analysis-ID$ID-$DATE"
##########################################
# make tables
       if [[ $RUN == "MAKETABLES" ]]
       then
	  ./CTA.MSCW_ENERGY.sub_make_tables.sh tables_CTA-$S-ID$ID-$TDATE $ID $ARRAY onSource $S $QSUBOPT
	  ./CTA.MSCW_ENERGY.sub_make_tables.sh tables_CTA-$S-ID$ID-$TDATE $ID $ARRAY cone $S $QSUBOPT
	  continue
##########################################
# analyse with lookup tables
       elif [[ $RUN == "ANATABLES" ]]
       then
	  TABLE="tables_CTA-$S-ID$ID-$TDATE-onAxis"
	  echo $TABLE
	  ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TABLE $ID $ARRAY $S $MSCWSUBDIRECTORY onSource $QSUBOPT
	  TABLE="tables_CTA-$S-ID$ID-$TDATE"
	  echo $TABLE
	  ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TABLE $ID $ARRAY $S $MSCWSUBDIRECTORY cone $QSUBOPT
	  continue
        fi

##########################################
# loop over all observation times
      for ((o = 0; o < ${#OBSTIME[@]}; o++ ))
      do
        OOTIME=${OBSTIME[$o]}

##########################################
# loop over all shower directions
       for ((a = 0; a < ${#MCAZ[@]}; a++ ))
       do
          AZ=${MCAZ[$a]}
# set run parameter file
	  PARA="$PDIR/scriptsInput.prod2.Erec$EREC.ID$ID$AZ.$S$AZ.runparameter"
	  rm -f $PARA
	  touch $PARA
	  echo "WRITING PARAMETERFILE $PARA"
	  NTYPF=NIM$NIMAGESMIN
	  EFFDIR="EffectiveArea-"$OOTIME"-Erec$EREC-ID$ID$AZ-$NTYPF-$DATE"
	  echo "MSCWSUBDIRECTORY $MSCWSUBDIRECTORY" >> $PARA
	  echo "TMVASUBDIR BDT-Erec$EREC-ID$ID$AZ-$NTYPF-$DATE" >> $PARA
	  echo "EFFAREASUBDIR $EFFDIR" >> $PARA
	  echo "RECID $ID" >> $PARA
	  echo "ENERGYRECONSTRUCTIONMETHOD $EREC" >> $PARA
	  echo "NIMAGESMIN $NIMAGESMIN" >> $PARA
	  echo "OBSERVINGTIME_H $OOTIME" >> $PARA
	  EFFDIR="/lustre/fs9/group/cta/users/maierg/CTA/analysis/AnalysisData/$S/$EFFDIR/"
##########################################
# train BDTs   
	  if [[ $RUN == "TRAIN" ]]
	  then
	    echo "$AZ " 
	     ./CTA.TMVA.sub_train.sh $ARRAY onSource $S $PARA $AZ $QSUBOPT
	     ./CTA.TMVA.sub_train.sh $ARRAY cone $S $PARA $AZ $QSUBOPT
##########################################
# IRFs: angular resolution
	  elif [[ $RUN == "ANGRES" ]]
	  then
            ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.TMVAFixedSignal $PARA AngularResolution $S 2 $AZ $QSUBOPT
##########################################
# IRFs: effective areas after quality cuts
	  elif [[ $RUN == "QC" ]]
	  then
	    ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.QC $PARA QualityCuts001CU $S 0 $AZ $QSUBOPT
##########################################
# IRFs: particle number files
	  elif [[ $RUN == "PARTFIL" ]]
	  then
	     ./CTA.ParticleRateWriter.sub.sh $ARRAY $EFFDIR/QualityCuts001CU cone $ID $EFFDIR/AngularResolution $QSUBOPT
##########################################
# IRFs: effective areas after gamma/hadron cuts
	  elif [[ $RUN == "CUTS" ]]
	  then
	    ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.TMVA $PARA BDT.W2.$DATE $S 0 $AZ $QSUBOPT
##########################################
# CTA WP Phys files
	  elif [[ $RUN == "PHYS" ]]
	  then
	    ./CTA.WPPhysWriter.sub.sh $ARRAY $EFFDIR/BDT.W2.$DATE $OOTIME DESY.$DATE.Erec$EREC.W2.ID$ID$AZ$NTYPF.$S 0 $ID $S $QSUBOPT
# unknown run set
	  elif [[ $RUN != "EVNDISP" ]]
	  then
	      echo "Unknown run set: $RUN"
	      exit
	  fi
      done
     done
   done
   echo 
   echo "(end of script)"
done
