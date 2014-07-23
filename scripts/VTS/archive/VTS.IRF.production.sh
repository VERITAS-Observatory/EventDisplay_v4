#!/bin/sh
#
# IRF production script (VERITAS)
#
###################################################################


if [ $# -ne 2 ]
then
   echo "./VTS.IRF.production.sh <run mode> <GRISU/CARE>"
   echo
   echo "  possible run modes are MAKETABLES ANALYSETABLES EFFECTIVEAREAS"
   echo
   exit
fi
RUN="$1"
SIMS="$2"

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
ATM=( "22" )
# table file
if [[ $SIMS == "GRISU" ]]
then
    TFIL="table_d20131115_GrIsuDec12_"
else
    TFIL="table_d20140416_CareJan1427_"
fi
# cut files
# $CUTFIL and $CUTS must be of same length
# CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate" "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft" "ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-SuperSoft" "ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open" )
# CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate" "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft" "ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open" )
# CUTS=( "med" "soft" "open" )
CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft" )
CUTS=( "soft" )
CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Ext-005CU-Soft" )
CUTS=( "softExt" )
CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Hard" )
CUTS=( "hard" )
CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate" )
CUTS=( "med" )
CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Ext-005CU-Moderate" )
CUTS=( "med" )
#CUTFIL=( "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate" "ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Hard" )
#CUTS=( "med" "hard" )

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
      T=$TFIL"ATM"$W"_"$A"_ID0"
######################################
# make tables
      if [[ $RUN == "MAKETABLES" ]]
      then
         ./VTS.MSCW_ENERGY.sub_make_tables.sh $T 0 $W $A $SIMS
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
            ./VTS.MSCW_ENERGY.sub_analyse_MC_VBF.sh $T $I $W $A $SIMS
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
               if [[ $SIMS == "GRISU" ]]
               then
                   D="/lustre/fs5/group/cta/VERITAS/analysis/EVDv400/"$A"_FLWO/mscw_ATM"$W"_d20131031"
               else
                   D="$VERITAS_DATA_DIR/analysis/EVDv400/"$A"_FLWO/mscw_CARE_ATM"$W"_d20140416"
               fi
               ./VTS.EFFAREA.sub_analyse.sh $C $SIMS-$A"-"$F"-ATM"$W"-ID"$I $I 1234 $D $SIMS
            done
         fi

      done

   done
done

