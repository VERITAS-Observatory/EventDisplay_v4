#!/bin/bash
# calculate mean tzeros
# Author: Gernot Maier

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
EVNDISP special-purpose analysis: calculate mean tzeros from a data file

VTS.EVNDISP.analyse_tzeros <run number> [teltoana]

required parameters:

    <run number>            calculate tzeros for this run

optional parameters:

    [teltoana]              use 1 for T1 only, 13 for T1 and T3 only, etc.
                            (default telescope combination is 1234)

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
RUNNUM=$1
[[ "$2" ]] && TELTOANA=$2 || TELTOANA="1234"
if [[ $TELTOANA == "-1" ]]; then
    TELTOANA="1234"
fi

# Check if source vbf file exists
SF=`find -L $VERITAS_DATA_DIR/data -name "$RUNNUM.cvbf"`
if [ ${#SF} = 0 ]; then
    echo "ERROR: VERITAS source file $RUNNUM.cvbf not found in $VERITAS_DATA_DIR/data/"
    exit
fi

# run options
OPT="-runmode=7 -runnumber=$RUNNUM -teltoana=$TELTOANA -readcalibdb"

# Run evndisp
echo "$EVNDISPSYS/bin/evndisp $OPT"
$EVNDISPSYS/bin/evndisp $OPT

exit
