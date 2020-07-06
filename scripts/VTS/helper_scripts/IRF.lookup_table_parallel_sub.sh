#!/bin/bash
# script to run over all zenith angles and telescope combinations and create lookup tables 

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
ZA=ZENITHANGLE
WOBBLE=WOBBLEOFFSET
NOISE=SIMNOISE
EPOCH=ARRAYEPOCH
ATM=ATMOSPHERE
RECID=RECONSTRUCTIONID
SIMTYPE=SIMULATIONTYPE
INDIR=INPUTDIR
ODIR=OUTPUTDIR
#SSCALING=SIZESCALING

TABFILE="table_${SIMTYPE}_${ZA}deg_${WOBBLE}wob_noise${NOISE}_${EPOCH}_ATM${ATM}_ID${RECID}"

# remove existing log and table file
rm -f "$ODIR/$TABFILE.root"
rm -f "$ODIR/$TABFILE.log"

# make the table part
$EVNDISPSYS/bin/mscw_energy -filltables=1 -limitEnergyReconstruction -write1DHistograms -inputfile "$INDIR/*[0-9].root" -tablefile "$ODIR/$TABFILE.root" -ze=$ZA -arrayrecid=$RECID -woff=$WOBBLE  &> "$ODIR/$TABFILE.log"

exit
