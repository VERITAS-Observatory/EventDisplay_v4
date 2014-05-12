#!/bin/bash
# display data files with eventdisplay

## HARD-CODED VALUES
# evndisp reconstruction runparamter
ACUT="EVNDISP.reconstruction.SW18_noDoublePass.runparameter"
ACUT="EVNDISP.reconstruction.runparameter"
# run options
OPT="-display=1 -teltoana=$TEL -reconstructionparameter $ACUT -vbfnsamples -readCalibDB"
## END OF HARD-CODED VALUES

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
EVNDISP special-purpose analysis: display data file and write results to file

SPANALYSIS.evndisp_display.sh <sourcefile> [teltoana] [run number] [TARGET]
 [WOBBLENORTH] [WOBBLEEAST] [RAOFFSET]

required parameters:

    <sourcefile>            VERITAS data file (vbf or cvbf file)

optional parameter:

    [teltoana]              use 1 for T1 only, 13 for T1 and T3 only, etc.
                            (default telescope combination is 1234)
                            
    [run number]            run number (can differ from actual run number)
    
    [TARGET]                target name (Crab, Mrk421, 1es2344, lsi+61303)
                            (for more do 'evndisp -printtargets')
                            
    [WOBBLENORTH]           wobble offsets north (e.g. 0.5) or south (e.g. -0.5)
                            in units of degrees
    
    [WOBBLEEAST]            wobble offsets east (e.g. 0.5) or west (e.g. -0.5)
                            in units of degrees
    
    [RAOFFSET]              right ascension offset for off run
                            (e.g. 7.5 for off run 30 min later)

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
RUNFILE=$1
[[ "$2" ]] && TELTOANA=$2 || TELTOANA="1234"
if [[ $TELTOANA == "-1" ]]; then
    TELTOANA="1234"
fi
[[ "$3" ]] && OPT="$OPT -runnumber=$3"
[[ "$4" ]] && OPT="$OPT -target $4"
[[ "$5" ]] && OPT="$OPT -wobblenorth=$5"
[[ "$6" ]] && OPT="$OPT -wobbleeast=$6"
[[ "$7" ]] && OPT="$OPT -raoffset=$7"

# Check if source file exists
if [[ ! -f $RUNFILE ]]; then
    echo "ERROR: VERITAS source file $RUNFILE not found"
    exit 1
fi

# Set remaining run options
OPT="$OPT -sourcefile $RUNFILE -teltoana=$TELTOANA"

# Run evndisp
echo "$EVNDISPSYS/bin/evndisp $OPT"
$EVNDISPSYS/bin/evndisp $OPT 

exit
