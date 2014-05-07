#!/bin/bash
# script to analyse VTS raw files (VBF) with eventdisplay
# Author: Gernot Maier
#
# 2014-04-16 (GM) checked

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
RUN=RUNFILE
CALIB=CALIBRATIONOPTION
ODIR=OUTPUTDIRECTORY
VPM=USEVPMPOINTING
MSCWDIR=MSCWDIRECTORY
LOGDIR="$ODIR"

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
    $EVNDISPSYS/bin/evndisp \
        -runnumber=$RUN     \
        -runmode=1          \
        &> $LOGDIR/$RUN.ped.log
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
    $EVNDISPSYS/bin/evndisp \
        -runnumber=$RUN     \
        -runmode=7          \
        ${OPT[@]}           \
        &> $LOGDIR/$RUN.tzero.log
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
# OPT+=( -calibrationfile calibrationlist.dat )
#
## double pass correction
# OPT+=( -nodp2005 )

# only applied if USEFROGS=true or 0
if [ $USEFROGS ] ; then
    OPT+=( -frogs $MSCWDIR/$RUN.mscw.root -frogid 0 )
fi

if [[ "$FASTDEVMODE" == "yes" ]] ; then
    echo "Warning, \$FASTDEVMODE=yes, so only processing the first 2000 events."
    OPT+=( -nevents=2000 )
fi

#########################################
# run eventdisplay
rm -f $LOGDIR/$RUN.log
$EVNDISPSYS/bin/evndisp             \
    -runnumber=$RUN                 \
    -reconstructionparameter $ACUTS \
    -outputfile $TEMPDIR/$RUN.root  \
    ${OPT[@]}                       \
    &> $LOGDIR/$RUN.log
echo "RUN$RUN EVNLOG $LOGDIR/$RUN.log"

# move data file from tmp dir to data dir
cp -f -v $TEMPDIR/$RUN.root $ODIR/$RUN.root
echo "RUN$RUN DATAOUT $ODIR/$RUN.root"
rm -f $TEMPDIR/$RUN.root

exit
