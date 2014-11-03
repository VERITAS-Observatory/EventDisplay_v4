#!/bin/bash
# calculate pedestals

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
EVNDISP special-purpose analysis: calculate pedestals from a data file
                                  (for high and low gain)

SPANALYSIS.evndisp_pedestal_events.sh <run number> [lowgain]

required parameters:

    <run number>        calculate pedestals for this run

optional parameters:

    [lowgain]           low gain pedestal calculation, either 64 or 128 samples
                        (expert user only!)

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
RUNNUM=$1
[[ "$2" ]] && LOWGAIN=$2 || LOWGAIN="0"

# Set proper options for low gain or high gain analysis
if [[ $LOWGAIN == "64" ]]; then
    # low gain run with 64 samples
    OPT="-calibrationsumwindow=20 -calibrationsumfirst=40 -runmode=6 -reconstructionparameter EVNDISP.reconstruction.LGCalibration.runparameter"
elif [[ $LOWGAIN == "128" ]]; then
    # low gain run with 128 samples
    OPT="-calibrationsumwindow=20 -calibrationsumfirst=100 -runmode=6 -reconstructionparameter EVNDISP.reconstruction.LGCalibration.runparameter"
elif [[ $LOWGAIN == "100" ]]; then
    # low gain run with 100 samples
    OPT="-calibrationsumwindow=20 -calibrationsumfirst=75 -runmode=6 -reconstructionparameter EVNDISP.reconstruction.LGCalibration.runparameter"
elif [[ $LOWGAIN == "0" ]]; then
    OPT="-runmode=1 -calibrationsumwindow=20 -calibrationsumfirst=0"
else
    echo "Invalid low-gain parameter; using default high-gain options"
    OPT="-runmode=1 -calibrationsumwindow=20 -calibrationsumfirst=0"
fi

# Check if source file exists
SF=`find -L $VERITAS_DATA_DIR/data -name "$RUNNUM.cvbf"`
if [[ ${#SF} = 0 ]]; then
   echo "ERROR: VERITAS source (VBF) file $RUNNUM.cvbf not found in $VERITAS_DATA_DIR/data/"
   exit 1
fi

# Run evndisp
echo "$EVNDISPSYS/bin/evndisp -runnumber=$RUNNUM $OPT"
$EVNDISPSYS/bin/evndisp -runnumber=$RUNNUM $OPT

exit
