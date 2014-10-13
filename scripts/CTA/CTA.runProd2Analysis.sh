#!/bin/sh
#
# analysis submission for production 2 (prod2) analysis
#
# this script is optimized for the DESY analysis
#
##############################################


if [ $# -ne 2 ] 
then
   echo 
   echo "./CTA.runProd2Analysis.sh <N/S/SM/P1> <run mode>"
   echo
   echo "  N=prod2-North, S=prod2-South, SM=prod2-South-merged P1=prod1"
   echo
   echo "  possible run modes are EVNDISP MAKETABLES ANATABLES TRAIN ANGRES QC PARTFIL CUTS PHYS "
   echo
   exit
fi
P2="$1"
RUN="$2"

#####################################
# qsub options
#   _M_ = -; _X_ = " "
QSUBOPT="_M_P_X_cta_high"
QSUBOPT="_M_P_X_cta_high_X__M_js_X_5000"

#####################################
# output directory for script parameter files
PDIR="$CTA_USER_LOG_DIR/tempRunParameterDir/"
mkdir -p $PDIR

#####################################
# analysis dates and table dates

TDATE="d20140309"
DATE="d20140718"

#####################################
# reconstruction IDs
RECID="0"

#####################################
# shower directions
#
# _180deg = south
# _0deg = north
MCAZ=( "_180deg" "_0deg" "" )

#####################################
# sites & sub array lists

##########################################################
# SOUTH
if [[ $P2 == "S" ]]
then
##########################################################
# data sets without trgmask files
#   SITE=( "prod2-Aar2014-NS" "prod2-LeoncitoPP-NS" "prod2-SAC100-lowE-NS" "prod2-SAC084-lowE-NS"  "prod2-Aar-500m-NS" )
##########################################################
# 40 deg data sets 
# (change run list directories!)
#   SITE=( "prod2-Aar-40deg-NS" "prod2-Leoncito-40deg-NS" "prod2-LeoncitoPP-40deg-NS" )
##########################################################
# NSB data sets
# (change run list directories!)
#   SITE=( "CL5040-prod2-Leoncito-NSB-x1.00-NS" "CL5040-prod2-Leoncito-NSB-x1.30-NS" "CL5040-prod2-Leoncito-NSB-x1.50-NS" )
# data sets with trgmask files
#   SITE=( "prod2-SAC100-NS" "prod2-SAC084-NS" "prod2-Leoncito-NS" )
##########################################################
# super-seeded Aar 20deg data set (without trgmask file)
#   SITE=( "prod2-Aar-lowE-NS" )
# super-seeded Aar 20deg data set
#   SITE=( "prod2-Aar-NS" )
##########################################################
# reduced arrays data sets
   SITE=( "prod2-LeoncitoPP-NS" )
   ARRAY="subArray.2a.list"
##########################################################
# SOUTH - merged simulation files
elif [[ $P2 == "SM" ]]
then
   SITE="prod2-Aar2014-NS"
   ARRAY="subArray.SM.dat"
##########################################################
# NORTH
elif [[ $P2 == "N" ]]
then
   SITE=( "prod2-US-NS" "prod2-SPM-NS" "prod2-Tenerife-NS" )
   SITE=( "prod2-SPM-NS" "prod2-Tenerife-NS" )
   SITE=( "prod2-SPM-NS" )
   ARRAY="subArray.2NN-fullList.list"
   ARRAY="subArray.2NN-LG.list"
   ARRAY="subArray.2NN.list"
############################
# prod1
elif [[ $P2 == "P1" ]]
then
  SITE=( "prod1-cta-ultra3" )
  ARRAY=( "subArray.I-noLST.list" )
  RECID="2"
  DATE="d20130415"
  MCAZ=( "" )
############################
# prod2-MS (trigsim)
elif [[ $P2 == "MS" ]]
then
  SITE=( "MS-TrigSimProd2SumD" )
  ARRAY=( "subArray.MS.list" )
  MCAZ=( "" )
else
   echo "error: unknown site; allowed are N or S"
   echo $P2
   exit
fi

#####################################
# particle types
PARTICLE=( "gamma_onSource" "gamma_cone" "electron" "proton" )

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
OBSTIME=( "100h" "500h" "1000h" )
OBSTIME=( "50h" "5h" "30m" "10m" "1m" "20s" )
OBSTIME=( "50h" "40h" "30h" "25h" "20h" "15h" "10h" "5h" )
OBSTIME=( "50h" )


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
# 20140313: this is partly broken...(works for Lenocito)
   if [[ $S == "prod2-Aar-NS" ]]
   then
      TRG="/lustre/fs13/group/cta/prod2/Aar/simtel/trgmask/"
   elif [[ $S == "prod2-Leoncito-NS" ]]
   then
      TRG="/lustre/fs16/group/cta/prod2/Leoncito/trgmask/"
# OLD SIMULATIONS WITH WRONG L1 TRIGGER
#      TRG="/lustre/fs13/group/cta/prod2/Leoncito/simtel/trgmask/"
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
          # run list for 40 deg files on dcache
	  LIST=/afs/ifh.de/group/cta/scratch/maierg/LOGS/CTA/runLists/prod2/40deg/$S.$N"".dcache.list
          # run list for 40 deg files on lustre
	  LIST=/afs/ifh.de/group/cta/scratch/maierg/LOGS/CTA/runLists/prod2/40deg/$S.$N"".grid.list.fullList
          # run list for 20 deg high NSB files on lustre
	  LIST=/afs/ifh.de/group/cta/scratch/maierg/LOGS/CTA/runLists/prod2/NSB/$S.$N"_20deg".list
          # run list for 20 deg files on lustre (default data sets)
	  LIST=/afs/ifh.de/group/cta/scratch/maierg/LOGS/CTA/runLists/prod2/$S.$N"_20deg".list
          echo $LIST

          # analysis of merged evndisp files
          if [[ $P2 == "SM" ]]
          then
               ./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob.prod2_merge.sh $ARRAY $LIST $N $S 0 $i $QSUBOPT
          # standard evndisplay analysis
          else
               ./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob.prod2.sh $ARRAY $LIST $N $S 0 $i $QSUBOPT $TRG
          fi 
       done
       continue
    fi
##########################################
# loop over all reconstruction IDs
    for ID in $RECID
    do
       MSCWSUBDIRECTORY="Analysis-ID$ID-$TDATE"
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
          echo "GETXOFFYOFFAFTERCUTS yes" >> $PARA
	  EFFDIR="/lustre/fs9/group/cta/users/maierg/CTA/analysis/AnalysisData/$S/$EFFDIR/"
##########################################
# train BDTs   
# (note: BDT training does not need to be done for all observing periods)
	  if [[ $RUN == "TRAIN" ]]
	  then
	    echo "$AZ " 
	     ./CTA.TMVA.sub_train.sh $ARRAY onSource $S $PARA $QSUBOPT $AZ
	     ./CTA.TMVA.sub_train.sh $ARRAY cone $S $PARA $QSUBOPT $AZ
##########################################
# IRFs: angular resolution
	  elif [[ $RUN == "ANGRES" ]]
	  then
            ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.TMVAFixedSignal $PARA AngularResolution $S 2 $QSUBOPT $AZ
##########################################
# IRFs: effective areas after quality cuts
	  elif [[ $RUN == "QC" ]]
	  then
	    ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.QC $PARA QualityCuts001CU $S 0 $QSUBOPT $AZ
##########################################
# IRFs: particle number files
	  elif [[ $RUN == "PARTFIL" ]]
	  then
	     ./CTA.ParticleRateWriter.sub.sh $ARRAY $EFFDIR/QualityCuts001CU cone $ID $EFFDIR/AngularResolution $QSUBOPT
##########################################
# IRFs: effective areas after gamma/hadron cuts
	  elif [[ $RUN == "CUTS" ]]
	  then
	    ./CTA.EFFAREA.subAllParticle_analyse.sh $ARRAY ANASUM.GammaHadron.TMVA $PARA BDT.L1.$DATE $S 0 $QSUBOPT $AZ
##########################################
# CTA WP Phys files
	  elif [[ $RUN == "PHYS" ]]
	  then
	    ./CTA.WPPhysWriter.sub.sh $ARRAY $EFFDIR/BDT.L1.$DATE $OOTIME DESY.$DATE.Erec$EREC.L1.ID$ID$AZ$NTYPF.$S 1 $ID $S $QSUBOPT
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
