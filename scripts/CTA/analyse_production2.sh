#!/bin/sh
#
# analysis submission for production 2 analysis
#
#
##############################################


if [ $# -ne 1 ]
then
   echo "./prod2.sh <run mode>"
   echo
   echo "  possible run modes are EVNDISP MAKETABLES COMBINETABLE ANATABLES TRAIN ANGRES QC PARTFIL CUTS PHYS "
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
SITE=( "prod2-Aar-North" "prod2-Leoncito-North" "prod2-SAC084-North" "prod2-Aar-South" "prod2-Leoncito-South" "prod2-SAC084-South" "prod2-G-Leoncito-North" "prod2-G-Leoncito-South" "prod2-SAC100-North" "prod2-SAC100-South" )
# for all other analysis
SITE=( "prod2-Aar-North" "prod2-Leoncito-North" "prod2-SAC084-North" "prod2-Aar-South" "prod2-Leoncito-South" "prod2-SAC084-South" "prod2-SAC100-North" "prod2-SAC100-South" )
SITE=( "prod2-Aar-North" "prod2-Leoncito-North" "prod2-SAC084-North" "prod2-Aar-South" "prod2-SAC084-South" "prod2-SAC100-North" "prod2-SAC100-South" )
SITE=( "prod2-Aar-North" "prod2-Leoncito-North" "prod2-SAC084-North" "prod2-SAC100-North" )
SITE=( "prod2-SAC084-South" "prod2-SAC100-South" "prod2-Aar-South" "prod2-Leoncito-South" )

#####################################
# particle types
PARTICLE=( "gamma_onSource" "gamma_cone10" "electron" "proton" )

#####################################
# reconstruction IDs
RECID="0 1 2 3 4"
RECID="0 1"

#####################################
# energy reconstruction
# (default is method 1)
EREC="1"

#####################################
# observing time [h]
OBSTIME="50"

#####################################
# sub array lists
ARRAY="subArray.prod2red.list"
ARRAY="subArray.2a.list"

#####################################
# analysis dates
DATE="d20130717"
TDATE="d20130717"

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

	  ./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob.prod2.sh $ARRAY $LIST $N $S 0 $i $TRG
       done
       continue
    fi
##########################################
# loop over all reconstruction IDs
    for ID in $RECID
    do
# set run parameter file
       PARA="$PDIR/scriptsInput.prod2.Erec$EREC.ID$ID.$S.runparameter"
       rm -f $PARA
       touch $PARA
       echo "WRITING PARAMETERFILE $PARA"
       EFFDIR="EffectiveArea-"$OBSTIME"h-Erec$EREC-ID$ID-$DATE"
       echo "MSCWSUBDIRECTORY Analysis-ID$ID-$DATE" >> $PARA
       echo "TMVASUBDIR BDT-Erec$EREC-ID$ID-$DATE" >> $PARA
       echo "EFFAREASUBDIR $EFFDIR" >> $PARA
       echo "RECID $ID" >> $PARA
       echo "ENERGYRECONSTRUCTIONMETHOD $EREC" >> $PARA
       echo "NIMAGESMIN 2" >> $PARA
       echo "OBSERVINGTIME_H $OBSTIME" >> $PARA
       EFFDIR="/lustre/fs9/group/cta/users/maierg/CTA/analysis/AnalysisData/$S/$EFFDIR/"
# make tables
       if [[ $RUN == "MAKETABLES" ]]
       then
	  ./CTA.MSCW_ENERGY.sub_make_tables.sh tables_CTA-$S-ID$ID-$TDATE $ID $ARRAY onSource $S
	  ./CTA.MSCW_ENERGY.sub_make_tables.sh tables_CTA-$S-ID$ID-$TDATE $ID $ARRAY cone10 $S
# combine tables
       elif [[ $RUN == "COMBINETABLE" ]]
       then
	   ./CTA.MSCW_ENERGY.combine_tables.sh tables_CTA-$S-ID$ID-$TDATE $ARRAY tables_CTA-$S-ID$ID-$DATE $CTA_USER_DATA_DIR/analysis/AnalysisData/$S/Tables/ $S
	   mv -v -i $CTA_USER_DATA_DIR/analysis/AnalysisData/Tables/tables_CTA-$S-ID$ID-$TDATE*.root $CTA_EVNDISP_AUX_DIR/Tables/
# analyse with lookup tables
       elif [[ $RUN == "ANATABLES" ]]
       then
	  TABLE="tables_CTA-$S-ID$ID-$TDATE"
	  if [[ $S == "prod2-G-Leoncito-North" ]]
	  then
	     TABLE="tables_CTA-prod2-Leoncito-North-ID$ID-$TDATE"
	  elif [[ $S == "prod2-G-Leoncito-South" ]]
	  then
	     TABLE="tables_CTA-prod2-Leoncito-South-ID$ID-$TDATE"
	  fi
	  echo $TABLE
	  ./CTA.MSCW_ENERGY.sub_analyse_MC.sh $TABLE $ID $ARRAY $S $PARA
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
	 ./CTA.WPPhysWriter.sub.sh $ARRAY $EFFDIR/BDT.$DATE $OBSTIME DESY.$DATE.Erec$EREC.ID$ID.$S 0 $ID $S
# unknown run set
       elif [[ $RUN != "EVNDISP" ]]
       then
	   echo "Unknown run set: $RUN"
	   exit
       fi
   done
   echo 
   echo
done
