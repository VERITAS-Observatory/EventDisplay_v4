#!/bin/sh
#
# analysis submission for production 2 analysis
#
#
##############################################


if [ $# -ne 1 ]
then
   echo "./analyse_production2.sh <run mode>"
   echo
   echo "  possible run modes are EVNDISP MAKETABLES ANATABLES TRAIN ANGRES QC PARTFIL CUTS PHYS "
   echo
   exit
fi
RUN="$1"


#####################################
# output directory for script parameter files
PDIR="$CTA_USER_LOG_DIR/tempRunParameterDir/"
mkdir -p $PDIR

#####################################
# sites
# for evndisp and MSCW analysis

SITE=( "prod2-Leoncito-NS" "prod2-Aar-NS" "prod2-SAC100-NS" "prod2-SAC084-NS" )
SITE=( "prod2-Aar-NS" "prod2-Aar-lowE-NS" )
SITE=( "prod2-Leoncito-lowE-NS" "prod2-Leoncito-NS" )
SITE=( "prod2-US-NS" )
SITE=( "prod2-US-NS" "prod2-SPM-NS" "prod2-Tenerife-NS" )
SITE=( "prod2-Aar-NS" "prod2-SAC100-NS" "prod2-SAC084-NS" "prod2-Leoncito-lowE-NS" "prod2-Aar-lowE-NS" "prod2-SAC100-lowE-NS" "prod2-SAC084-lowE-NS" )
SITE=( "prod2-Aar-NS" "prod2-SAC100-NS" "prod2-SAC084-NS" "prod2-Leoncito-lowE-NS" "prod2-Aar-lowE-NS" "prod2-SAC100-lowE-NS" "prod2-SAC084-lowE-NS" "prod2-Leoncito-NS" )
SITE=( "prod2-SAC084-lowE-NS" )

#####################################
# particle types
PARTICLE=( "gamma_onSource" "gamma_cone" "electron" "proton" )

#####################################
# shower directions
#
# _180deg = south
# _0deg = north
MCAZ=( "" )
MCAZ=( "_180deg" "_0deg" "" )

#####################################
# reconstruction IDs
RECID="0 1 2 3 4"
RECID="0"

#####################################
# energy reconstruction
# (default is method 1)
EREC="1"

#####################################
# cut on number of images and number 
# of images per telescope type
NTYPEMIN=( "2" "4" "2" )
NTYPEMIN=( "2" "4" "4" )
NTYPEMIN=( "3" "4" "4" )
NTYPEMIN=( "0" "0" "0" )
NTYPEMIN=( "2" "2" "2" )
NIMAGESMIN="2"

#####################################
# observing time [h]
OBSTIME=( "5h" "30m" "10m" "1m" "20s" )
OBSTIME=( "50h" "5h" "30m" "10m" "1m" "20s" )
OBSTIME=( "50h" )

#####################################
# sub array lists
ARRAY="subArray.2aF1.list"

#####################################
# analysis dates
DATE="d20130830"
TDATE="d20130830"
TDATE="d20130812"

NSITE=${#SITE[@]}
for (( m = 0; m < $NSITE ; m++ ))
do
   S=${SITE[$m]}

   echo
   echo "======================================================================================"
   echo "SITE: $S"
   echo "RUN: $RUN"

   if [[ $S == *Aar* ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/Aar/simtel/trgmask/"
   elif [[ $S == *Leoncito* ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/Leoncito/simtel/trgmask/"
   elif [[ $S == *SAC084* ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/SAC084/"
   elif [[ $S == *SAC100* ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/SAC100/"
   fi

# eventdisplay
    if [[ $RUN == "EVNDISP" ]]
    then

# loop over all particle types
      for ((i = 0; i < ${#PARTICLE[@]}; i++ ))
      do
	  N=${PARTICLE[$i]}
	  LIST=/afs/ifh.de/group/cta/scratch/maierg/LOGS/CTA/runLists/prod2/$S.$N"_20deg".list

	 if [[ $S == *lowE* ]] || [[ $S == "prod2-US-NS" ]]
	 then
	     ./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob.prod2.sh $ARRAY $LIST $N $S 0 $i
         else
	     ./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob.prod2.sh $ARRAY $LIST $N $S 0 $i $TRG
         fi
       done
       continue
    fi
##########################################
# loop over all reconstruction IDs
    for ID in $RECID
    do
       MSCWSUBDIRECTORY="Analysis-ID$ID-$DATE"
# make tables
       if [[ $RUN == "MAKETABLES" ]]
       then
	  ./CTA.MSCW_ENERGY.sub_make_tables.sh tables_CTA-$S-ID$ID-$TDATE $ID $ARRAY onSource $S
	  ./CTA.MSCW_ENERGY.sub_make_tables.sh tables_CTA-$S-ID$ID-$TDATE $ID $ARRAY cone $S
	  continue
# analyse with lookup tables
       elif [[ $RUN == "ANATABLES" ]]
       then
	  TABLE="tables_CTA-$S-ID$ID-$TDATE-onAxis"
	  echo $TABLE
	  ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TABLE $ID $ARRAY $S $MSCWSUBDIRECTORY onSource
	  TABLE="tables_CTA-$S-ID$ID-$TDATE"
	  echo $TABLE
	  ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TABLE $ID $ARRAY $S $MSCWSUBDIRECTORY cone
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
	  for ((t = 0; t < ${#NTYPEMIN[@]}; t++ ))
	  do
	     NTYPF=$NTYPF"-TYP"$t"N"${NTYPEMIN[t]}
	  done
	  EFFDIR="EffectiveArea-"$OOTIME"-Erec$EREC-ID$ID$AZ-$NTYPF-$DATE"
	  echo "MSCWSUBDIRECTORY $MSCWSUBDIRECTORY" >> $PARA
	  echo "TMVASUBDIR BDT-Erec$EREC-ID$ID$AZ-$NTYPF-$DATE" >> $PARA
	  echo "EFFAREASUBDIR $EFFDIR" >> $PARA
	  echo "RECID $ID" >> $PARA
	  echo "ENERGYRECONSTRUCTIONMETHOD $EREC" >> $PARA
	  echo "NIMAGESMIN $NIMAGESMIN" >> $PARA
	  for ((t = 0; t < ${#NTYPEMIN[@]}; t++ ))
	  do
	     echo NTYPEMIN_$t ${NTYPEMIN[t]} >> $PARA
          done
	  echo "OBSERVINGTIME_H $OOTIME" >> $PARA
	  EFFDIR="/lustre/fs9/group/cta/users/maierg/CTA/analysis/AnalysisData/$S/$EFFDIR/"
# train BDTs   
	  if [[ $RUN == "TRAIN" ]]
	  then
	    echo "$AZ " 
	     ./CTA.TMVA.sub_train.sh $ARRAY onSource $S $PARA $AZ
	     ./CTA.TMVA.sub_train.sh $ARRAY cone $S $PARA $AZ
# IRFs: angular resolution
	  elif [[ $RUN == "ANGRES" ]]
	  then
            ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.TMVAFixedSignal $PARA AngularResolution $S 2 $AZ
# IRFs: effective areas after quality cuts
	  elif [[ $RUN == "QC" ]]
	  then
	    ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.QC $PARA QualityCuts001CU $S 0 $AZ
# IRFs: particle number files
	  elif [[ $RUN == "PARTFIL" ]]
	  then
	     ./CTA.ParticleRateWriter.sub.sh $ARRAY $EFFDIR/QualityCuts001CU cone $ID $EFFDIR/AngularResolution
# IRFs: effective areas after gamma/hadron cuts
	  elif [[ $RUN == "CUTS" ]]
	  then
	    ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.TMVA $PARA BDT.N1.$DATE $S 0 $AZ
# CTA WP Phys files
	  elif [[ $RUN == "PHYS" ]]
	  then
	    ./CTA.WPPhysWriter.sub.sh $ARRAY $EFFDIR/BDT.N1.$DATE $OOTIME DESY.$DATE.Erec$EREC.N1.ID$ID$AZ$NTYPF.$S 1 $ID $S
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
