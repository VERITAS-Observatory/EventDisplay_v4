#!/bin/bash
# script to analyse MC files with lookup tables
# Author: Gernot Maier

# qsub parameters
h_cpu=00:29:00; h_vmem=6000M; tmpdir_size=100G

if [ $# -lt 5 ]; then
# begin help message
echo "
IRF generation: analyze simulation ROOT files using mscw_energy 

IRF.mscw_energy_MC.sh <table file> <epoch> <atmosphere> <Rec ID> <sim type>
 [particle]

required parameters:

    <table file>            mscw_energy lookup table file
    
    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.
    
    <sim type>              original VBF file simulation type (e.g. GRISU, CARE)

optional parameters:
    
    [particle]              type of particle used in simulation:
                            gamma = 1, proton = 14, alpha (helium) = 402
                            (default = 1  -->  gamma)
                            
Note: zenith angles, wobble offsets, and noise values are hard-coded into script

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
TABFILE=$1
TABFILE=${TABFILE%%.root}.root
EPOCH=$2
ATM=$3
RECID=$4
SIMTYPE=$5
[[ "$6" ]] && PARTICLE=$6 || PARTICLE=1

# zenith angles/noise/wobble offsets
if [[ $SIMTYPE == "GRISU" ]]; then
    # GrISU simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
    NOISE_LEVELS=( 075 100 150 200 250 325 425 550 750 1000 )
    WOBBLE_OFFSETS=( 0.00 0.25 0.50 0.75 1.00 1.25 1.50 1.75 2.00 )
else
    # CARE simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 )
    NOISE_LEVELS=( 50 80 120 170 230 290 370 450 )
    WOBBLE_OFFSETS=( 0.5 )
fi

# Particle names
PARTICLE_NAMES=( [1]=gamma [2]=electron [14]=proton [402]=alpha )
PARTICLE_TYPE=${PARTICLE_NAMES[$PARTICLE]}

# Check that table file exists
if [[ "$TABFILE" == `basename $TABFILE` ]]; then
    TABFILE="$VERITAS_EVNDISP_AUX_DIR/Tables/$TABFILE"
fi
if [ ! -f "$TABFILE" ]; then
    echo "Error, table file not found, exiting..."
    exit 1
fi

# input directory containing evndisp_MC products
if [[ -z $VERITAS_IRF_ANA_DIR ]]; then
    INDIR="$VERITAS_IRF_ANA_DIR/$EDVERSION/${PARTICLE_TYPE}_${EPOCH}_ATM${ATM}_${SIMTYPE}"
elif [[ ! -z $VERITAS_IRF_ANA_DIR || ! -d $INDIR ]]; then
    INDIR="$VERITAS_DATA_DIR/analysis/$EDVERSION/${PARTICLE_TYPE}_${EPOCH}_ATM${ATM}_${SIMTYPE}"
elif [[ ! -d $INDIR ]]; then
    echo "Error, could not locate input directory. Locations searched:"
    echo "$VERITAS_IRF_ANA_DIR/$EDVERSION/${PARTICLE_TYPE}_${EPOCH}_ATM${ATM}_${SIMTYPE}"
    echo "$VERITAS_DATA_DIR/analysis/$EDVERSION/${PARTICLE_TYPE}_${EPOCH}_ATM${ATM}_${SIMTYPE}"
    exit 1
fi
echo "Input file directory: $INDIR"

# output directory for mscw_energy_MC products
ODIR="$INDIR/RecID$RECID"
echo "Output file directory: $ODIR"
mkdir -p $ODIR

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.ANATABLES/"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.mscw_energy_MC_sub.sh"

######################################################
# loop over all zenith angles/noise/wobble offsets
for ZA in ${ZENITH_ANGLES[@]}; do
    for WOBBLE in ${WOBBLE_OFFSETS[@]}; do
        for NOISE in ${NOISE_LEVELS[@]}; do
            echo "Now processing zenith angle $ZA, wobble $WOBBLE, noise level $NOISE"

            # make run script
            FSCRIPT="$LOGDIR/$EPOCH-$RECID-$ZENITH-$WOBBLE-$NOISE-$PARTICLE"
            sed -e "s|INPUTDIR|$INDIR|" \
                -e "s|OUTPUTDIR|$ODIR|" \
                -e "s|TABLEFILE|$TABFILE|" \
                -e "s|ZENITHANGLE|$ZA|" \
                -e "s|NOISELEVEL|$NOISE|" \
                -e "s|WOBBLEOFFSET|$WOBBLE|" \
                -e "s|ATMOSPHERE|$ATM|" \
                -e "s|ARRAYEPOCH|$EPOCH|" \
                -e "s|PARTICLETYPE|$PARTICLE|" \
                -e "s|SIMULATIONTYPE|$SIMTYPE|" \
                -e "s|RECONSTRUCTIONID|$RECID|" $SUBSCRIPT.sh > $FSCRIPT.sh

            # run locally or on cluster
            SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
            SUBC=`eval "echo \"$SUBC\""`
            if [[ $SUBC == *qsub* ]]; then
                $SUBC $FSCRIPT.sh
            elif [[ $SUBC == *parallel* ]]; then
                echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
            fi
        done
    done
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
