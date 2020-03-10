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
NROOTFILES=NFILES
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
	 if [[ $NROOTFILES == 1 ]]; then
		  rm -f $OSUBDIR/$OFILE.log
		  inputfilename="$INDIR/*[0-9].root"
		  outputfilename="$DDIR/$OFILE.mscw.root"
		  logfile="$OSUBDIR/$OFILE.log"
	 elif	[[ $NROOTFILES > 1 ]]; then
		  ITER=$((SGE_TASK_ID - 1))
		  rm -f $OSUBDIR/${OFILE}_$ITER.log
		  inputfilename="$INDIR/*[0-9]_$ITER.root"
		  outputfilename="$DDIR/${OFILE}_$ITER.mscw.root"
		  logfile="$OSUBDIR/${OFILE}_$ITER.log"
	 fi
	 $EVNDISPSYS/bin/mscw_energy $MOPT -inputfile $inputfilename -outputfile $outputfilename -noise=$NOISE &> $logfile
# cp results file back to data directory and clean up
	 outputbasename=$( basename $outputfilename )
    cp -f -v $outputfilename $OSUBDIR/$outputbasename
    rm -f $outputfilename
    rmdir $DDIR
    chmod g+w $OSUBDIR/$outputbasename
    chmod g+w $logfile
done

exit
