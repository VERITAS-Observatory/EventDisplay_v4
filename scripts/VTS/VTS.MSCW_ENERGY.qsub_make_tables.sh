#$ -S /bin/bash
#
# script to run over all zenith angles and telescope combinations and create lookup tables 
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

# subdirectory with date where evndisp simulation files are found
DSET="analysis_d20131031"

# directory with input file
DDIR="$VERITAS_DATA_DIR/analysis/EVDv400/"$ARRAY"_FLWO/gamma_"$IZE"deg_750m/wobble_"$WOFF"/"$DSET"_ATM"$ATMO"_"$ANAC"_NOISE"$NOISEX"/*.root"

# remove existing log and table file
rm -f $ODDIR/$TFIL-NOISE$NOISEY-$IZE-$WOFF.root
rm -f $ODDIR/$TFIL-NOISE$NOISEY-$IZE-$WOFF.log

# make the tables
$EVNDISPSYS/bin/mscw_energy -filltables=1 -inputfile "$DDIR" -tablefile $ODDIR/$TFIL-NOISE$NOISEY-$IZE-$WOFF.root -ze=$IZE -arrayrecid=$RECID -woff=$WOFF -noise=$NOISEY > $LOGDIR/$TFIL-NOISE$NOISEY-$IZE-$WOFF.log

exit
