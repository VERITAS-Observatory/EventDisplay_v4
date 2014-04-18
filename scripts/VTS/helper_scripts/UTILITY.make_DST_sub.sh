#!/bin/bash
# script to make DSTs
# Author: Gernot Maier
# edited by G Hughes to make summation window a variable

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
RUN=RUNFILE
PED=PEDESTALS
SUMW=SUMWINDOW

# temporary (scratch) directory
if [[ -n $TMPDIR ]]; then
    TEMPDIR=$TMPDIR/$RUN
else
    TEMPDIR="$VERITAS_USER_DATA_DIR/TMPDIR"
fi
echo "Temporary directory: $TEMPDIR"
mkdir -p $TEMPDIR

# output data files are written to this directory
ODIR="$VERITAS_USER_DATA_DIR/analysis/EVD400_DST/$SUMW/"
mkdir -p $ODIR

# output log files are written to this directory
LOGDIR="$VERITAS_USER_LOG_DIR/analysis/EVD400_DST/$SUMW/"
mkdir -p $LOGDIR

# check for the existence of a run parameter file corresponding
# to the given summation window; if it exists, remove it and replace it
# with the LOWGAIN runparameter file, modified for the given sum window
RUNPDIR="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/"
RUNPFILE="EVNDISP.reconstruction.SW${SUMW}_noDoublePass.runparameter"
if [ -e "$RUNPDIR/$RUNPFILE" ]; then
    rm "$RUNPDIR/$RUNPFILE"
fi

cat $RUNPDIR/EVNDISP.reconstruction.LOWGAIN.runparameter | sed s|"XX"|"$SUMW"|g > $RUNPDIR/$RUNPFILE

# eventdisplay reconstruction parameter
ACUTS=$RUNPFILE

#########################################
# pedestal and tzero calculation
if [[ $PED == "1" ]]; then
    rm -f $LOGDIR/$RUN.ped.log
    $EVNDISPSYS/bin/evndisp -runnumber=$RUN -runmode=1 &> $LOGDIR/$RUN.ped.log
    
    rm -f $LOGDIR/$RUN.tzero.log
    $EVNDISPSYS/bin/evndisp -runnumber=$RUN -runmode=7 -nocalibnoproblem &> $LOGDIR/$RUN.tzero.log
fi

#########################################
# run eventdisplay
rm -f $LOGDIR/$RUN.log
$EVNDISPSYS/bin/evndisp -runnumber=$RUN -runmode=4 -nevents=5000 -nocalibnoproblem -reconstructionparameter $ACUTS -dstfile $TEMPDIR/$RUN.DST.root &> $LOGDIR/$RUN.DST.log

# move data file from temp dir to data dir
cp -f -v $TEMPDIR/$RUN.DST.root $ODIR/$RUN.DST.root
rm -f $TEMPDIR/$RUN.DST.root

# remove run parameter file
rm -f $RUNPDIR/$RUNPFILE

exit
