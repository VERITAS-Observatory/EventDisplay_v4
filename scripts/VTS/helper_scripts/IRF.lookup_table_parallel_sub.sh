#!/bin/bash
# script to run over all zenith angles and telescope combinations and create lookup tables 

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
ZA=ZENITHANGLE
WOBBLE=WOBBLEOFFSET
SIMNOISE=SIMNOISE
TABNOISE=TABLENOISE
EPOCH=ARRAYEPOCH
ATM=ATMOSPHERE
RECID=RECONSTRUCTIONID
SIMTYPE=SIMULATIONTYPE
INDIR=INPUTDIR
ODIR=OUTPUTDIR

TABFILE="table_${SIMTYPE}_${ZA}deg_${WOBBLE}wob_${TABNOISE}noise_${EPOCH}_ATM${ATM}_ID${RECID}"

# temporary directory
if [[ -n "$TMPDIR" ]]; then 
    DDIR="$TMPDIR/MSCW_${ZA}deg_${WOBBLE}deg_NOISE${NOISE}_ID${RECID}"
else
    DDIR="/tmp/MSCW_${ZA}deg_${WOBBLE}deg_NOISE${NOISE}_ID${RECID}"
fi
mkdir -p $DDIR
echo "Temporary directory: $DDIR"

# remove existing log and table file
rm -f "$ODIR/$TABFILE.root"
rm -f "$ODIR/$TABFILE.log"

# make the table part
$EVNDISPSYS/bin/mscw_energy -filltables=1 -inputfile "$INDIR/*[0-9].root" -tablefile "$ODIR/$TABFILE.root" -ze=$ZA -arrayrecid=$RECID -woff=$WOBBLE -noise=$TABNOISE &> "$ODIR/$TABFILE.log"

# cp results file back to data directory and clean up
cp -f -v $DDIR/$TABFILE.root $ODIR/$TABFILE.root
rm -f $DDIR/$TABFILE.root
rmdir $DDIR

exit
