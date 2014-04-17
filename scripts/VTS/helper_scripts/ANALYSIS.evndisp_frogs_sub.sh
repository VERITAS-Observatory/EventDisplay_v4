#!/bin/bash
# script to analyse VTS files using FROGS
# Author: Gernot Maier

# parameters replaced by parent script using sed
RUN=RUNFILE
CALIB=CALIBRATIONOPTION
ODIR=OUTPUTDIRECTORY
MSCWDIR=MSCWDIRECTORY
VPM=USEVPMPOINTING
LOGDIR="$ODIR"

source $EVNDISPSYS/setObservatory.sh VTS

# temporary (scratch) directory
if [[ -n $TMPDIR ]]; then
    TEMPDIR=$TMPDIR/$RUN
else
    TEMPDIR="$VERITAS_USER_DATA_DIR/TMPDIR"
fi
mkdir -p $TEMPDIR

# eventdisplay reconstruction parameter
ACUTS="EVNDISP.reconstruction.runparameter"

#########################################
# pedestal calculation
if [[ $CALIB == "1" || $CALIB == "2" ]]; then
    rm -f $LOGDIR/$RUN.ped.log
    $EVNDISPSYS/bin/evndisp -runnumber=$RUN -runmode=1 &> $LOGDIR/$RUN.ped.log
fi

#########################################
# read gain and toffsets from VOFFLINE DB
OPT="-readCalibDB"

# None of the following command line options is needed for the standard analysis!

## Read gain and toff from VOFFLINE DB requiring a special version of analysis 
# OPT = "-readCalibDB version_number"
## Warning: this version must already exist in the DB

## Read gain and toff from VOFFLINE DB and save the results in the directory
## where the calib file should be (it won't erase what is already there)
# OPT = "-readandsavecalibdb"

## Quick look option (has no effect if readCalibDB or equivalent is set):
## use this option when you don't care about the calibration information
## if no gain can be read from your laser file, gain will be set to 1
## if no toffset can be read from your laser file, toffset will be set to 0
# OPT="-nocalibnoproblem"
## Note: if this option is NOT set, the analysis will break if there is problem
## reading the gain and toffset files

#########################################
# Command line options for pointing and calibration

# pointing from pointing monitor (DB)
if [[ $VPM == "1" ]]; then
    OPT="$OPT -usedbvpm"
fi

## pointing from db using T-point correction from 2007-11-05
# OPT="$OPT -useDBtracking -useTCorrectionfrom "2007-11-05""
## OFF data run
# OPT="$OPT -raoffset=6.25"
## use text file for calibration information
# OPT="$OPT -calibrationfile calibrationlist.dat"
## double pass correction
# OPT="$OPT -nodp2005"

#########################################

OPT="$OPT -frogs $MSCWDIR/$RUN.mscw.root -frogid 0"

#########################################
# run eventdisplay
rm -f $LOGDIR/$RUN.log
$EVNDISPSYS/bin/evndisp -runnumber=$RUN -reconstructionparameter $ACUTS -outputfile $TEMPDIR/$RUN.root $OPT &> $LOGDIR/$RUN.log

# move data file from tmp dir to data dir
cp -f -v $TEMPDIR/$RUN.root $ODIR/$RUN.root
rm -f $TEMPDIR/$RUN.root

exit
