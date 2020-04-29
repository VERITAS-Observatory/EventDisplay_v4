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
SSCALING=SIZESCALING
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
    MOPT="-noNoTrigger -nomctree -writeReconstructedEventsOnly=1 -arrayrecid=$ID -tablefile $TABFILE"
# use short output tree (use -noshorttree for Data/MC comparison)
    # MOPT="-shorttree $MOPT"
    echo "MSCW options: $MOPT"

# run mscw_energy
    rm -f $OSUBDIR/$OFILE.log
    rm -f $OSUBDIR/$OFILE.list
    ls -1 $INDIR/*[0-9].root > $OSUBDIR/$OFILE.list
    outputfilename="$DDIR/$OFILE.mscw.root"
    logfile="$OSUBDIR/$OFILE.log"
    $EVNDISPSYS/bin/mscw_energy $MOPT -inputfilelist $OSUBDIR/$OFILE.list -outputfile $outputfilename -noise=$NOISE &> $logfile
# cp results file back to data directory and clean up
    outputbasename=$( basename $outputfilename )
    cp -f -v $outputfilename $OSUBDIR/$outputbasename
    rm -f $outputfilename
    rmdir $DDIR
    chmod g+w $OSUBDIR/$outputbasename
    chmod g+w $logfile
done

exit
