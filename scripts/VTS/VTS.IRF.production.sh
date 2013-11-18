#!/bin/sh
#
# IRF production script (VERITAS)
#
# (early version with debugging statements)
#
###################################################################


if [ $# -ne 1 ]
then
   echo "./VTS.IRF.production.sh <run mode>"
   echo
   echo "  possible run modes are MAKETABLES ANALYSETABLES EFFECTIVEAREAS"
   echo
   exit
fi
RUN="$1"

#####################################################################
# FULL sets

# VERITAS epoch
ARRAY=( "V6" "V5" "V4" )
ARRAY=( "V5" )
# reconstruction IDs (=4 and 3-telescope combinations)
ID=( "0" "1" "2" "3" "4" )
ID=( "0" )
# atmospheres
ATM=( "21" "22" )
ATM=( "21" )
# table file
TABILEFIL="table_d20131115_GrIsuDec12_"
# cut files
# $CUTFIL and $CUTS must be of same length
# CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate" "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft" "ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-SuperSoft" "ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open" )
CUTS=( "med" "soft" "supersoft" "open" )
CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate" "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft" "ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open" )
CUTS=( "med" "soft" "open" )

#########################
# loop over all epochs
NEPOCH=${#ARRAY[@]}
for ((e = 0; e < $NEPOCH; e++ ))
do
   A=${ARRAY[$e]}

#########################
# loop over all atmospheres
   NATM=${#ATM[@]}
   for ((w = 0; w < $NATM; w++ ))
   do
      W=${ATM[$w]}

# lookup table
      T=$TFIL$W"_ATM"$A"_ID0.root"
######################################
# make tables
      if [[ $RUN == "MAKETABLES" ]]
      then
         echo "  ./VTS.MSCW_ENERGY.sub_make_tables.sh $T 0 $W $A"
      fi

######################################
# loop over all array layouts
      NID=${#ID[@]}
      for ((i = 0; i < $NID; i++ ))
      do
         I=${ID[$i]}
######################################
# analyse table files
         if [[ $RUN == "ANALYSETABLES" ]]
         then
            echo "./VTS.MSCW_ENERGY.sub_analyse_MC_VBF.sh $T $I $W $A"
         fi
######################################
# analyse effective areas
         if [[ $RUN == "EFFECTIVEAREAS" ]]
         then
            NCUT=${#CUTFIL[@]}
            for ((c = 0; c < $NCUT; c++))
            do
               C=${CUTFIL[$c]}
               F=${CUTS[$c]}
               D="$VERITAS_DATA_DIR/analysis/EVDv400/"$A"_FLWO/mscw_ATM"$W"_d20131031"
               ./VTS.EFFAREA.sub_analyse.sh $C $A"-"$F"-ATM"$W"-ID"$I $I 1234 $D
            done
         fi

      done

   done
done

