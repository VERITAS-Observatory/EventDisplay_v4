#! /bin/bash
#
# script to run over all zenith angles and telescope combinations and create lookup tables 
#
# Revision $Id: qsub_make_tables.sh,v 1.1.2.2.4.1.2.3.2.1.6.3.2.4.2.8.2.5.6.2 2011/04/18 07:48:17 gmaier Exp $
#
# Author: Gernot Maier
#

TFIL=TABLEFILE
ANAC=TELESCOPES
RECID=RECONSTRUCTIONID
NOISEX=NOISEI
NOISEY=NOISET
ARRAY=AAAAA
ATMO=BBBBBBB
LOGDIR=CCCCCC
ODDIR=DDDDDD

source $EVNDISPSYS/setObservatory.sh VERITAS

# this is where the executable can be found
cd $EVNDISPSYS/bin

# zenith angle bins
# IZE=( 00 20 30 35 40 45 50 55 60 65 ) 
IZE=( 00 20 30 35 40 )
NZE=${#IZE[@]}
# wobble off bins
# WOFF=( 0.00 0.25 0.5 0.75 1.00 1.25 1.50 1.75 2.00 )
WOFF=( 0.5 )
NWOF=${#WOFF[@]}

##############################################
# clean up disk
N=0
while [ $N -lt $NZE ]
do
    W=0
    while [ $W -lt $NWOF ]
    do
       rm -f $ODDIR/$TFIL-NOISE$NOISEX-${WOFF[$W]}.root
       rm -f $LOGDIR/$TFIL-NOISE$NOISEX-${WOFF[$W]}.log
       let "W = $W + 1"
   done
   let "N = $N + 1"
done

##############################################
# loop over all zenith angle bins
N=0
while [ $N -lt $NZE ]
do
##############################################
# loop over all wobble offsets
    echo "ZE BIN $N $NZE ${IZE[$N]}"
    W=0
    while [ $W -lt $NWOF ]
    do
      echo "   WOFF BIN $W $NWOF ${WOFF[$W]}"

# directory with input file
      DDIR="$VERITAS_DATA_DIR/analysis/EVDv400/"$ARRAY"_FLWO/gamma_"${IZE[$N]}"deg_750m/wobble_"${WOFF[$W]}"/analysis_d20120326_ATM"$ATMO"_"$ANAC"_NOISE"$NOISEX"/*.root"

# make the tables
      $EVNDISPSYS/bin/mscw_energy -filltables=1 -inputfile "$DDIR" -tablefile $ODDIR/$TFIL-NOISE$NOISEX-${WOFF[$W]}.root -ze=${IZE[$N]} -arrayrecid=$RECID -woff=${WOFF[$W]} -noise=$NOISEY >> $LOGDIR/$TFIL-NOISE$NOISEX-${WOFF[$W]}.log

      let "W = $W + 1"
   done
   let "N = $N + 1"
done
