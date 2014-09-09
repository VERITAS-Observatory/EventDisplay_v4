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
RECID=RECONSTRUCTIONID

ITER=$((SGE_TASK_ID - 1))

# file names
OFILE="${ZA}deg_${WOBBLE}wob_NOISE${NOISE}_${ITER}"

# temporary directory
if [[ -n "$TMPDIR" ]]; then 
    DDIR="$TMPDIR/MSCW_${ZA}deg_${WOBBLE}deg_NOISE${NOISE}_ID${RECID}"
else
    DDIR="/tmp/MSCW_${ZA}deg_${WOBBLE}deg_NOISE${NOISE}_ID${RECID}"
fi
mkdir -p $DDIR
echo "Temporary directory: $DDIR"

# mscw_energy command line options
MOPT="-noNoTrigger -noshorttree -nomctree -writeReconstructedEventsOnly=1 -arrayrecid=$RECID -tablefile $TABFILE"
# use short output tree
# MOPT="-shorttree $MOPT"

# run mscw_energy
rm -f $ODIR/$OFILE.log
###$EVNDISPSYS/bin/mscw_energy $MOPT -inputfile "$INDIR/*[0-9].root" -outputfile "$DDIR/$OFILE.mscw.root" -noise=$NOISE &> $ODIR/$OFILE.log
$EVNDISPSYS/bin/mscw_energy $MOPT -inputfile "$INDIR/*[0-9]_$ITER.root" -outputfile "$DDIR/$OFILE.mscw.root" -noise=$NOISE &> $ODIR/$OFILE.log
# cp results file back to data directory and clean up
cp -f -v $DDIR/$OFILE.mscw.root $ODIR/$OFILE.mscw.root
rm -f $DDIR/$OFILE.mscw.root
rmdir $DDIR

exit
