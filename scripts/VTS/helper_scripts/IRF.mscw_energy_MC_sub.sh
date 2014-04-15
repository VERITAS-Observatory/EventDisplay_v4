#!/bin/bash
# script to analyse MC files with lookup tables
# Author: Gernot Maier

# parameters replaced by parent script using sed
INDIR=INPUTDIR
ODIR=OUTPUTDIR
TABFILE=TABLEFILE
ZA=ZENITHANGLE
NOISE=NOISELEVEL
WOBBLE=WOBBLEOFFSET
ATM=ATMOSPHERE
EPOCH=ARRAYEPOCH
PARTICLE=PARTICLETYPE
SIMTYPE=SIMULATIONTYPE
RECID=RECONSTRUCTIONID
TELTOANA="1234"

# file names
INFILE="${SIMTYPE}_${PARTICLE}_${ZA}deg_${WOBBLE}wob_NOISE${NOISE}_${EPOCH}_ATM${ATM}"
OFILE="${SIMTYPE}_${PARTICLE}_${ZA}deg_${WOBBLE}wob_NOISE${NOISE}_${EPOCH}_ATM${ATM}_ID${RECID}"

# temporary directory
if [[ -z $TMPDIR ]]; then 
    DDIR="$TMPDIR/MSCW_${ZA}deg_${WOBBLE}deg_NOISE${NOISE}_ID${RECID}"
else
    DDIR="/tmp/MSCW_${ZA}deg_${WOBBLE}deg_NOISE${NOISE}_ID${RECID}"
fi
mkdir -p $DDIR
echo "Temporary directory: $DDIR"

# mscw_energy command line options
MOPT="-noNoTrigger -nomctree -writeReconstructedEventsOnly=1 -arrayrecid=$RECID -tablefile $TABFILE"
# use short output tree
MOPT="-shorttree $MOPT"

# run mscw_energy
rm -f $ODIR/$OFILE.log
$EVNDISPSYS/bin/mscw_energy $MOPT -inputfile "$INDIR/$INFILE.root" -outputfile "$DDIR/$OFILE.mscw.root" -noise=$NOISE &> $ODIR/$OFILE.log

# cp results file back to data directory and clean up
cp -f -v $DDIR/$OFILE.mscw.root $ODIR/$OFILE.mscw.root
rm -f $DDIR/$OFILE.mscw.root
rmdir $DDIR

exit
