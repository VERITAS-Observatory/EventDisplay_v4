#!/bin/bash
# script to run over all noise levels and create lookup tables (queue submit)

# qsub parameters
h_cpu=03:29:00; h_vmem=4000M; tmpdir_size=1G

if [ $# -ne 8 ]; then
# begin help message
echo "
IRF generation: create partial (for one point in the parameter space) lookup tables from MC evndisp ROOT files

IRF.generate_lookup_table_parts.sh <epoch> <atmosphere> <zenith> <offset angle> <NSB level> <pedvar level> <Rec ID> <sim type>

required parameters:
        
    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
                            
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)

    <zenith>                zenith angle of simulations [deg]

    <offset angle>          offset angle of simulations [deg]

    <NSB level>             NSB level of simulations [MHz]

    <pedvar level>          Average pedvar for this NSB level
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
    
    <sim type>              original VBF file simulation type (e.g. GRISU, CARE)
    
--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
EPOCH=$1
ATM=$2
ZA=$3
WOBBLE=$4
NOISE=$5
PEDVAR=$6
RECID=$7
SIMTYPE=$8
PARTICLE_TYPE="gamma"

# EventDisplay version
EDVERSION=`$EVNDISPSYS/bin/evndisp --version | tr -d .`

# input directory containing evndisp products
if [[ -n "$VERITAS_IRFPRODUCTION_DIR" ]]; then
    INDIR=$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/${SIMTYPE}/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}"/ze"$ZA"deg_offset"$WOBBLE"deg_NSB"$NOISE"MHz"
fi
if [[ ! -d $INDIR ]]; then
    echo "Error, could not locate input directory. Locations searched:"
    echo "$INDIR"
    exit 1
fi
echo "Input file directory: $INDIR"

# Output file directory
if [[ ! -z $VERITAS_IRFPRODUCTION_DIR ]]; then
    ODIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/${SIMTYPE}/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}/Tables"
fi
echo "Output file directory: $ODIR"
mkdir -p $ODIR

# run scripts and output are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.MAKETABLES"
mkdir -p $LOGDIR

SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.lookup_table_parallel_sub"

# loop over all zenith angles, wobble offsets, and noise bins
echo "Processing Zenith = $ZA, Wobble = $WOBBLE, Noise = $NOISE"

FSCRIPT="$LOGDIR/$EPOCH-MK-TBL.$DATE.MC-$ZA-$WOBBLE-$NOISE-$EPOCH-$ATM-$RECID"
rm -f $FSCRIPT.sh

sed -e "s|ZENITHANGLE|$ZA|" \
    -e "s|WOBBLEOFFSET|$WOBBLE|" \
    -e "s|SIMNOISE|$NOISE|" \
    -e "s|TABLENOISE|$PEDVAR|" \
    -e "s|ARRAYEPOCH|$EPOCH|" \
    -e "s|ATMOSPHERE|$ATM|" \
    -e "s|RECONSTRUCTIONID|$RECID|" \
    -e "s|SIMULATIONTYPE|$SIMTYPE|" \
    -e "s|INPUTDIR|$INDIR|" \
    -e "s|OUTPUTDIR|$ODIR|" $SUBSCRIPT.sh > $FSCRIPT.sh

chmod u+x $FSCRIPT.sh

# run locally or on cluster
SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
SUBC=`eval "echo \"$SUBC\""`
if [[ $SUBC == *qsub* ]]; then
    $SUBC $FSCRIPT.sh
elif [[ $SUBC == *parallel* ]]; then
    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
fi

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
