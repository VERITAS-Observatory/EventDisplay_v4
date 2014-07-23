#!/bin/sh
#
# script to analyse MC files with lookup tables
#
# Author: Gernot Maier
#
##########################################################################################################

if  [ $# -lt 5 ]
then
   echo
   echo "VTS.MSCW_ENERGY.sub_analyse_MC_VBF.sh <table file> <recid> <atm (21/22)> <array (V4/V5/V6)> <GRISU/CARE> [gamma/proton/helium]" 
   echo
   echo "analyse MC data (loop over zenith angles, noise levels and wobble offsets)"
   echo "<table file>       table file name (without .root and full path)"
   echo "optional: [helium/proton/helium (default=gamma)]" 
   echo
   echo $#
   exit
fi

##############################################
# input variables
##############################################
TFIL=$1
ANAC="1234"
RECID=$2
ATMO=$3
ARRAY=$4
PART="gamma"
SIM=$5
if [ -n "$6" ]
then
   PART=$6
fi

FSCRIPT="VTS.MSCW_ENERGY.qsub_analyse_MC_VBF"

##############################################
# ze/noise/wobble offsets
##############################################
# zenith angles
# wobble offsets
# noise levels (package dependent)
if [ $SIM = "GRISU" ]
then
    RUNN="1"
    ZE=( 20 00 30 35 40 45 50 55 60 65 )
    NOISE=( 075 100 150 200 250 325 425 550 750 1000 )
    WOBBLE=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
# CARE
else
    RUNN="9"
    ZE=( 20 00 30 35 40 45 50 55 60 65 )
    NOISE=(  50  80 120 170 230 290  370  450 )
    WOBBLE=( 0.5 )
fi
NNOISE=${#NOISE[@]}
NZE=${#ZE[@]}
NWOBBLE=${#WOBBLE[@]}


##############################################
# directory for run scripts
##############################################
DATE=`date +"%y%m%d"`
FDIR=$VERITAS_USER_LOG_DIR"/"$DATE/MSCW.ANATABLES/
mkdir -p $FDIR
# directory for run log files
# LDIR=/dev/null
LDIR=$FDIR

##############################################
# loop over all ze/noise/wobble offsets
##############################################
for ((i=0; i < $NZE; i++))
do
   IZE=${ZE[$i]}

   for ((n=0; n < $NNOISE; n++))
   do
     NNOI=${NOISE[$n]}

     for ((w=0; w < $NWOBBLE; w++))
     do
       WOFF=${WOBBLE[$w]}

      FNAM="$ARRAY-$FSCRIPT-$ANAC-$RECID-$IZE-$NNOI-$WOFF-$PART"
      echo $FDIR/$FNAM

# pass command line options to submission script
      sed -e "s|TABLEFILE|$TFIL|" \
          -e "s/TELESCOPES/$ANAC/" \
	  -e "s/IZENITH/$IZE/" \
	  -e "s/NNNNOISE/$NNOI/" \
	  -e "s/ATMOOOS/$ATMO/" \
	  -e "s/WWWOBB/$WOFF/" \
	  -e "s/PAAAAART/$PART/" \
	  -e "s/RUUUUNNN/$RUNN/" \
	  -e "s/ARRRRAY/$ARRAY/" \
          -e "s/SIMS/$SIM/" \
	  -e "s/RECONSTRUCTIONID/$RECID/" $FSCRIPT.sh > $FDIR/$FNAM.sh

# submit the job
      qsub -l os=sl6 -l h_cpu=00:29:00 -l h_vmem=6000M -l tmpdir_size=100G  -V -o $LDIR -e $LDIR $FDIR/$FNAM.sh
     done
   done
done

exit

