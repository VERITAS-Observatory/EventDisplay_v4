#!/bin/bash
# script to analyse MC files with lookup tables

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
INDIR=INPUTDIR
ODIR=OUTPUTDIR
TABFILE=TABLEFILE
ZA=ZENITHANGLE
NOISE=NOISELEVEL
WOBBLE=WOBBLEOFFSET
RECID="RECONSTRUCTIONID"

#loop over all reconstruction IDs
for ID in $RECID; do

# output directory
    OSUBDIR="$ODIR/MSCW_RECID$ID"
    mkdir -p $OSUBDIR
    chmod g+w $OSUBDIR
    echo "Output directory for data products: " $OSUBDIR

# file names
    OFILE="${ZA}deg_${WOBBLE}wob_NOISE${NOISE}"

# temporary directory
    if [[ -n "$TMPDIR" ]]; then 
        DDIR="$TMPDIR/MSCW_${ZA}deg_${WOBBLE}deg_NOISE${NOISE}_ID${ID}"
    else
        DDIR="/tmp/MSCW_${ZA}deg_${WOBBLE}deg_NOISE${NOISE}_ID${ID}"
    fi
    mkdir -p $DDIR
    echo "Temporary directory: $DDIR"

# mscw_energy command line options
    MOPT="-noNoTrigger -shorttree -nomctree -writeReconstructedEventsOnly=1 -arrayrecid=$ID -tablefile $TABFILE"
# use short output tree
    # MOPT="-shorttree $MOPT"
    echo "MSCW options: $MOPT"

# run mscw_energy
    rm -f $OSUBDIR/$OFILE.log
    $EVNDISPSYS/bin/mscw_energy $MOPT -inputfile "$INDIR/*[0-9].root" -outputfile "$DDIR/$OFILE.mscw.root" -noise=$NOISE &> $OSUBDIR/$OFILE.log

# cp results file back to data directory and clean up
    cp -f -v $DDIR/$OFILE.mscw.root $OSUBDIR/$OFILE.mscw.root
    rm -f $DDIR/$OFILE.mscw.root
    rmdir $DDIR
    chmod g+w $OSUBDIR/$OFILE.mscw.root
    chmod g+w $OSUBDIR/$OFILE.log
done

exit
