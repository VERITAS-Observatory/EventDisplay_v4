#!/bin/bash
# script to analyse VTS raw files (VBF) with eventdisplay

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
RUN=RUNFILE
CALIB=CALIBRATIONOPTION
ODIR=OUTPUTDIRECTORY
VPM=USEVPMPOINTING
MODEL3D=USEMODEL3D
LOGDIR="$ODIR"

# temporary (scratch) directory
if [[ -n $TMPDIR ]]; then
    TEMPDIR=$TMPDIR/$RUN
else
    TEMPDIR="$VERITAS_USER_DATA_DIR/TMPDIR"
fi
echo "Scratch dir: $TEMPDIR"
mkdir -p $TEMPDIR

# eventdisplay reconstruction parameter
ACUTS="EVNDISP.reconstruction.runparameter"

#########################################
# pedestal calculation
if [[ $CALIB == "1" || $CALIB == "2" ]]; then
    rm -f $LOGDIR/$RUN.ped.log
    $EVNDISPSYS/bin/evndisp -runmode=1 -runnumber=$RUN -reconstructionparameter $ACUTS &> $LOGDIR/$RUN.ped.log
	echo "RUN$RUN PEDLOG $LOGDIR/$RUN.ped.log"
fi

#########################################
# read gain and toffsets from VOFFLINE DB
OPT=( "-readCalibDB" )

# None of the following command line options is needed for the standard analysis!

## Read gain and toff from VOFFLINE DB requiring a special version of analysis 
# OPT+=( -readCalibDB version_number )
## Warning: this version must already exist in the DB

## Read gain and toff from VOFFLINE DB and save the results in the directory
## where the calib file should be (it won't erase what is already there)
# OPT+=( -readandsavecalibdb )

## Quick look option (has no effect if readCalibDB or equivalent is set):
## use this option when you don't care about the calibration information
## if no gain can be read from your laser file, gain will be set to 1
## if no toffset can be read from your laser file, toffset will be set to 0
# OPT+=( -nocalibnoproblem )
## Note: if this option is NOT set, the analysis will break if there is problem
## reading the gain and toffset files

#########################################
# average tzero calculation
if [[ $CALIB == "1" || $CALIB == "3" ]]; then
    rm -f $LOGDIR/$RUN.tzero.log
    $EVNDISPSYS/bin/evndisp -runnumber=$RUN -runmode=7 -reconstructionparameter $ACUTS ${OPT[@]} &> $LOGDIR/$RUN.tzero.log
	echo "RUN$RUN TZEROLOG $LOGDIR/$RUN.tzero.log"
fi

#########################################
# Command line options for pointing and calibration

# pointing from pointing monitor (DB)
if [[ $VPM == "1" ]]; then
    OPT+=( -usedbvpm )
fi

## pointing from db using T-point correction from 2007-11-05
# OPT+=( -useDBtracking -useTCorrectionfrom "2007-11-05" )
#
## OFF data run
# OPT+=( -raoffset=6.25 )
#
## use text file for calibration information
if [[ "$SCIPIPE_MANUALLASER" == "yes" ]] ; then
	OPT+=( -calibrationfile "$SCIPIPE_MANUALLASERFILE" )
fi
#
## double pass correction
# OPT+=( -nodp2005 )

# Command line options for Model3D
if [[ $MODEL3D == "1" ]]; then
    OPT+=( -model3d -lnlfile "$VERITAS_EVNDISP_AUX_DIR/Tables/table_LnL.root" )
fi

if [[ "$SCIPIPE_FASTDEVMODE" == "yes" ]] ; then
	OPT+=( -nevents=2000 ) 
fi


#########################################
# run eventdisplay
LOGFILE="$LOGDIR/$RUN.log"
rm -f $LOGDIR/$RUN.log
$EVNDISPSYS/bin/evndisp -runnumber=$RUN -reconstructionparameter $ACUTS -outputfile $TEMPDIR/$RUN.root ${OPT[@]} &> "$LOGFILE"
echo "RUN$RUN EVNDISPLOG $LOGFILE"

# move data file from tmp dir to data dir
DATAFILE="$ODIR/$RUN.root"
cp -f -v $TEMPDIR/$RUN.root $DATAFILE
echo "RUN$RUN EVNDISPDATA $DATAFILE"
rm -f $TEMPDIR/$RUN.root

exit
