#!/bin/bash
# plot a MC VBF file using an external noise file

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
EVNDISP special-purpose analysis: display a simulation VBF file (with external
noise file)

SPANALYSIS.evndisp_display_MC_nonoise.sh <input VBF file> [run number]
 [noise level] [noise file]

required parameters:

    <input VBF file>        location of simulation VBF file

optional parameters:

    [run number]            run number associated with input VBF file
                            (default: 65432)
    
    [noise level]           noise level in GrISU units (default: 200)
    
    [noise file]            location of external GrISU noise file (default:
                            \$VERITAS_EVNDISP_AUX_DIR/NOISE/NOISExxx.grisu
                            where xxx = noise level)

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
SIMFILE="$1"
[[ "$2" ]] && RUNNUM=$2    || RUNNUM="65432"
[[ "$3" ]] && NOISELEV=$3  || NOISELEV="200"
[[ "$4" ]] && NOISEFILE=$4 || NOISEFILE="$VERITAS_EVNDISP_AUX_DIR/NOISE/NOISE$NOISELEV.grisu"

# Run options
OPT="-display=1 -runnumber=$RUNNUM -plotmethod=0 -sourcetype=2 -pedestalseed=1020 -pedestalnoiselevel=$NOISELEV"

# default noise level
PEDLEV="16."

# dead channel definition for MC
# DEAD="deadChannelDefinition_VERITAS_MC_d20101110.dat"

# array analysis cuts
# ACUTS="array_analysis_cuts_VERITAS_d20101110.dat"

# $EVNDISPSYS/bin/evndisp -sourcefile $SIMFILE -pedestalfile $NOISE -deadchannelfile $DEAD -pedestalDefaultPedestal=$PEDLEV -arraycuts $ACUTS $OPT
$EVNDISPSYS/bin/evndisp -nodp2005 -donotusedbinfo -sourcefile $SIMFILE -pedestalfile $NOISEFILE -pedestalDefaultPedestal=$PEDLEV $OPT

exit
