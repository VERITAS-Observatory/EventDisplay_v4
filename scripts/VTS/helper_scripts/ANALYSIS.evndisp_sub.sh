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
CALIBFILE=USECALIBLIST
TELTOANA=TELTOANACOMB
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
# DST ACUTS="EVNDISP.reconstruction.SW18_noDoublePass.runparameter"

#########################################
# pedestal calculation
if [[ $CALIB == "1" || ( $CALIB == "2" || $CALIB == "4" ) ]]; then
    rm -f $LOGDIR/$RUN.ped.log
    $EVNDISPSYS/bin/evndisp -runmode=1 -runnumber=$RUN -reconstructionparameter $ACUTS &> $LOGDIR/$RUN.ped.log
    echo "RUN$RUN PEDLOG $LOGDIR/$RUN.ped.log"
fi

#########################################

if [[ $CALIB == "4" ]]; then
	## use text file for calibration information
	OPT+=( -calibrationfile $CALIBFILE )
else
	# read gain and toffsets from VOFFLINE DB
	OPT+=( "-readCalibDB" )
fi

# restrict telescope combination to be analyzed:
if [[ $TELTOANA == "1234" ]]; then
	echo "Analyzed telescopes: $TELTOANA (all telescopes)"
else
	OPT+=( -teltoana=$TELTOANA )
fi

# None of the following command line options is needed for the standard analysis!

## Read gain and toff from VOFFLINE DB requiring a special version of analysis 
# OPT+=( -readCalibDB version_number )
## Warning: this version must already exist in the DB

## Read gain and toff from VOFFLINE DB and save the results in the directory
## where the calib file should be (it won't erase what is already there)
# OPT+=( -readandsavecalibdb )

#########################################
# average tzero calculation
if [[ $CALIB == "1" || ( $CALIB == "3" || $CALIB == "4" ) ]]; then
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
$EVNDISPSYS/bin/evndisp -runnumber=$RUN -noshorttree -reconstructionparameter $ACUTS -outputfile $TEMPDIR/$RUN.root ${OPT[@]} &> "$LOGFILE"
# DST $EVNDISPSYS/bin/evndisp -runnumber=$RUN -nevents=250000 -runmode=4 -readcalibdb -dstfile $TEMPDIR/$RUN.dst.root -reconstructionparameter $ACUTS -outputfile $TEMPDIR/$RUN.root ${OPT[@]} &> "$LOGFILE"
echo "RUN$RUN EVNDISPLOG $LOGFILE"

# move data file from tmp dir to data dir
DATAFILE="$ODIR/$RUN.root"
cp -f -v $TEMPDIR/$RUN.root $DATAFILE
echo "RUN$RUN EVNDISPDATA $DATAFILE"
rm -f $TEMPDIR/$RUN.root
# DST cp -f -v $TEMPDIR/$RUN.dst.root $DATAFILE

exit
