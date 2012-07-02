#!/bin/sh
#
# script to analyse MC files with lookup tables
#
# Revision $Id: analyse_MC.sh,v 1.1.2.3.4.1.12.2.6.2.2.2.6.2.2.2.2.12.2.7.6.3 2011/04/19 07:39:44 gmaier Exp $
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ]
then
   echo
   echo "VTS.MSCW_ENERGY.sub_analyse_MC_VBF.sh <table file> <recid> <atm (21/22)> <array (V4/V5) > [gamma/proton/helium]" 
   echo
   echo "analyse MC data (loop over zenith angles, noise levels and wobble offsets)"
   echo "<table file>       table file name (without .root and full path)"
   echo "optional: [helium/proton/helium (default=gamma)]" 
   exit
fi

##############################################
# input variables
##############################################
TFIL=$1
ANAC="1234"
RECID=$2
ATMO=$3
FTRE="FALSE"
ARRAY=$4
PART="gamma"
RUNN="1"
if [ -n "$5" ]
then
   PART=$5
fi

FSCRIPT="VTS.MSCW_ENERGY.qsub_analyse_MC_VBF"

##############################################
# ze/noise/wobble offsets
##############################################
# zenith angles
ZE=( 20 00 30 35 40 45 50 55 60 65 )
ZE=( 00 30 35 40 45 50 55 60 65 )
NZE=${#ZE[@]}
# noise levels
NOISE=( 075 100 150 200 250 325 425 550 750 1000 )
if [ $FTRE = "TRUE" ]
then
   NOISE=( 250 )
fi
NNOISE=${#NOISE[@]}
# wobble offsets
WOBBLE=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
NWOBBLE=${#WOBBLE[@]}


##############################################
# directory for run scripts
##############################################
FDIR=$VERITAS_USER_LOG_DIR"/queueMSCW/"
mkdir -p $FDIR

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

      FNAM="$FSCRIPT-$ANAC-$RECID-$IZE-$NNOI-$WOFF-$PART"
      echo $FNAM

      sed -e "s|TABLEFILE|$TFIL|" $FSCRIPT.sh > $FDIR/$FSCRIPT-1.sh
      sed -e "s/TELESCOPES/$ANAC/" $FDIR/$FSCRIPT-1.sh > $FDIR/$FSCRIPT-2.sh
      rm -f $FDIR/$FSCRIPT-1.sh
      sed -e "s/IZENITH/$IZE/" $FDIR/$FSCRIPT-2.sh > $FDIR/$FSCRIPT-2a.sh
      rm -f $FDIR/$FSCRIPT-2.sh
      sed -e "s/NNNNOISE/$NNOI/" $FDIR/$FSCRIPT-2a.sh > $FDIR/$FSCRIPT-2b.sh
      rm -f $FDIR/$FSCRIPT-2a.sh
      sed -e "s/ATMOOOS/$ATMO/" $FDIR/$FSCRIPT-2b.sh > $FDIR/$FSCRIPT-2c.sh
      rm -f $FDIR/$FSCRIPT-2b.sh
      sed -e "s/FUUUUUL/$FTRE/" $FDIR/$FSCRIPT-2c.sh > $FDIR/$FSCRIPT-2f.sh
      rm -f $FDIR/$FSCRIPT-2c.sh
      sed -e "s/WWWOBB/$WOFF/" $FDIR/$FSCRIPT-2f.sh > $FDIR/$FSCRIPT-3.sh
      rm -f $FDIR/$FSCRIPT-2f.sh
      sed -e "s/PAAAAART/$PART/" $FDIR/$FSCRIPT-3.sh > $FDIR/$FSCRIPT-3a.sh
      rm -f $FDIR/$FSCRIPT-3.sh
      sed -e "s/RUUUUNNN/$RUNN/" $FDIR/$FSCRIPT-3a.sh > $FDIR/$FSCRIPT-3b.sh
      rm -f $FDIR/$FSCRIPT-3a.sh
      sed -e "s/ARRRRAY/$ARRAY/" $FDIR/$FSCRIPT-3b.sh > $FDIR/$FSCRIPT-3c.sh
      rm -f $FDIR/$FSCRIPT-3b.sh
      sed -e "s/RECONSTRUCTIONID/$RECID/" $FDIR/$FSCRIPT-3c.sh > $FDIR/$FNAM.sh
      rm -f $FDIR/$FSCRIPT-3c.sh

      chmod u+x $FDIR/$FNAM.sh

# submit the job
      qsub -l os="sl*" -l h_cpu=00:29:00 -l h_vmem=6000M -l tmpdir_size=100G  -V -o $FDIR -e $FDIR $FDIR/$FNAM.sh
     done
   done
done

exit

