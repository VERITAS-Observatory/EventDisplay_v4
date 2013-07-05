#!/bin/sh
#
# analysis submission for production 2 analysis
#
#
##############################################


if [ $# -ne 1 ]
then
   echo "./prod2.sh <run mode>"
   exit
fi
RUN="$1"

#SITE=( "prod2-Aar-North" "prod2-Aar-South" "prod2-Leoncito-North" "prod2-Leoncito-South" "prod2-SAC084-North" "prod2-SAC084-South"  )
#SITE=( "prod2-Aar-South" "prod2-Leoncito-North" ) 
#SITE=( "prod2-Aar-North" "prod2-Aar-South" "prod2-Leoncito-North" "prod2-Leoncito-South" )
#SITE=( "prod2-Aar-North" "prod2-Aar-South" )
#SITE=( "prod2-Leoncito-North" "prod2-Leoncito-South" )
#SITE=( "prod2-Aar-North" "prod2-Aar-South" )

#SITE=( "prod2-G-Leoncito-North" )
#SITE=( "prod2-Aar-North" )
#SITE=( "prod2-Aar-North" "prod2-Leoncito-North" ) 
#SITE=( "prod2-SAC084-North" )
#SITE=( "prod2-Aar-South" "prod2-Aar-South" "prod2-SAC084-South" )
#SITE=( "prod2-Leoncito-North" ) 
#SITE=( "prod2-Aar-North" "prod2-Leoncito-North" "prod2-SAC084-North" )

SITE=( "prod2-Aar-North" "prod2-SAC084-North" )

PARTICLE=( "gamma_onSource" "gamma_cone10" "electron" "proton" )

RECID="0"
ARRAY="subArray.2a.list"
DATE="d20130702"

NSITE=${#SITE[@]}
for (( m = 0; m < $NSITE ; m++ ))
do
   S=${SITE[$m]}

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
   fi

# eventdisplay
    if [[ $RUN == "EVNDISP" ]]
    then

# loop over all particle types
      for ((i = 0; i < ${#PARTICLE[@]}; i++ ))
      do
	  N=${PARTICLE[$i]}
	  LIST=/afs/ifh.de/group/cta/scratch/maierg/LOGS/CTA/runLists/prod2/$S.$N"_20deg".list

	  ./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob.prod2.sh $ARRAY $LIST $N $S 0 $i $TRG
       done
       continue
    fi
##########################################
# loop over all reconstruction IDs
    for ID in $RECID
    do
       PARA="$CTA_EVNDISP_AUX_DIR/ParameterFiles/scriptsInput.prod2.ID$ID.runparameter"
       EFFDIR="/lustre/fs9/group/cta/users/maierg/CTA/analysis/AnalysisData/$S/EffectiveArea-ID$ID-$DATE/"
# make tables
       if [[ $RUN == "MAKETABLES" ]]
       then
	  ./CTA.MSCW_ENERGY.sub_make_tables.sh tables_CTA-$S-ID$ID-$DATE $ID $ARRAY onSource $S
	  ./CTA.MSCW_ENERGY.sub_make_tables.sh tables_CTA-$S-ID$ID-$DATE $ID $ARRAY cone10 $S
# combine tables
       elif [[ $RUN == "COMBINETABLE" ]]
       then
	   ./CTA.MSCW_ENERGY.combine_tables.sh tables_CTA-$S-ID$ID-$DATE $ARRAY tables_CTA-$S-ID$ID-$DATE $CTA_USER_DATA_DIR/analysis/AnalysisData/$S/Tables/ $S
# analyse with lookup tables
       elif [[ $RUN == "ANATABLES" ]]
       then
	  TABLE="tables_CTA-$S-ID$ID-$DATE"
	  if [[ $S == "prod2-G-Leoncito-North" ]]
	  then
	     TABLE="tables_CTA-prod2-Leoncito-North-ID$ID-$DATE"
	  elif [[ $S == "prod2-G-Leoncito-South" ]]
	  then
	     TABLE="tables_CTA-prod2-Leoncito-South-ID$ID-$DATE"
	  fi
	  echo $TABLE
	  ./CTA.MSCW_ENERGY.subAllParticle_analyse_MC.sh $TABLE $ID $ARRAY $S $PARA
# train BDTs   
       elif [[ $RUN == "TRAIN" ]]
       then
	  ./CTA.TMVA.sub_train.sh $ARRAY onSource $S $PARA
	  ./CTA.TMVA.sub_train.sh $ARRAY cone10 $S $PARA
# IRFs: angular resolution
       elif [[ $RUN == "ANGRES" ]]
       then
	 ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.TMVAFixedSignal $PARA AngularResolution $S 2
# IRFs: effective areas after quality cuts
       elif [[ $RUN == "QC" ]]
       then
	 ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.QC $PARA QualityCuts001CU $S
# IRFs: particle number files
       elif [[ $RUN == "PARTFIL" ]]
       then
	  ./CTA.ParticleRateWriter.sub.sh $ARRAY $EFFDIR/QualityCuts001CU cone10 $ID
# IRFs: effective areas after gamma/hadron cuts
       elif [[ $RUN == "CUTS" ]]
       then
	 ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.TMVA $PARA BDT.$DATE $S
# CTA WP Phys files
       elif [[ $RUN == "PHYS" ]]
       then
	 ./CTA.WPPhysWriter.sub.sh $ARRAY $EFFDIR/BDT.$DATE 50. DESY.$DATE.ID$ID.$S 1 $ID $S
# unknown run set
       elif [[ $RUN != "EVNDISP" ]]
       then
	   echo "Unknown run set: $RUN"
	   exit
       fi
   done
done
