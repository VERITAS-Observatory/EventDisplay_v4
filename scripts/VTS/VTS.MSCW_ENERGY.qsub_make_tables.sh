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
IZE=ZENITH
WOFF=WOBBLEOFFSET

source $EVNDISPSYS/setObservatory.sh VERITAS

# this is where the executable can be found
cd $EVNDISPSYS/bin

# directory with input file
DDIR="$VERITAS_DATA_DIR/analysis/EVDv400/"$ARRAY"_FLWO/gamma_"$IZE"deg_750m/wobble_"$WOFF"/analysisApr12_d20120901_ATM"$ATMO"_"$ANAC"_NOISE"$NOISEX"/*.root"

# remove existing log and table file
rm -f $ODDIR/$TFIL-NOISE$NOISEY-$IZE-$WOFF.root
rm -f $LOGDIR/$TFIL-NOISE$NOISEY-$IZE-${WOFF[$W]}.log

# make the tables
$EVNDISPSYS/bin/mscw_energy -filltables=1 -inputfile "$DDIR" -tablefile $ODDIR/$TFIL-NOISE$NOISEY-$IZE-$WOFF.root -ze=$IZE -arrayrecid=$RECID -woff=$WOFF -noise=$NOISEY > $LOGDIR/$TFIL-NOISE$NOISEY-$IZE-${WOFF[$W]}.log

exit
