#!/bin/bash
# submit evndisp for grisu simulations (analyse all noise levels at the same time)
# Author: Gernot Maier 

# qsub parameters
h_cpu=47:59:00; h_vmem=6000M; tmpdir_size=100G

if [ $# -lt 4 ]; then
# begin help message
echo "
IRF generation: analyze simulation VBF files using evndisp 

IRF.evndisp_MC.sh <sim directory> <epoch> <atmosphere> <sim type> [particle]
 [FROGS] [events]

required parameters:

    <sim directory>         directory containing simulation VBF files

    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)
    
    <sim type>              original VBF file simulation type (e.g. GRISU, CARE)

optional parameters:
    
    [particle]              type of particle used in simulation:
                            gamma = 1, electron = 2, proton = 14, helium = 402
                            (default = 1  -->  gamma)
    
    [FROGS]                 set to 1 to use FROGS (GrISU only! default: off)
    
    [events]                FROGS ONLY: number of events per division
                            (default: 5000000)
                            
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
SIMDIR=$1
EPOCH=$2
ATM=$3
SIMTYPE=$4
[[ "$5" ]] && PARTICLE=$5 || PARTICLE=1
[[ "$6" ]] && USEFROGS=$6 || USEFROGS=0
[[ "$7" ]] && NEVENTS=$7  || NEVENTS=5000000

if [[ $SIMTYPE = "GRISU" ]]; then
    # GrISU simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
    NOISE_LEVELS=( 075 100 150 200 250 325 425 550 750 1000 )
    WOBBLE_OFFSETS=( 0.00 0.25 0.50 0.75 1.00 1.25 1.50 1.75 2.00 )
elif [ $SIMTYPE = "CARE" ]; then
    # CARE simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 )
    NOISE_LEVELS=( 50 80 120 170 230 290 370 450 )
    WOBBLE_OFFSETS=( 0.5 )
else
    echo "Invalid simulation type. Exiting..."
    exit 1
fi

# Particle names
PARTICLE_NAMES=( [1]=gamma [2]=electron [14]=proton [402]=alpha )
PARTICLE_TYPE=${PARTICLE_NAMES[$PARTICLE]}

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EVNDISP.ANAMCVBF"
mkdir -p $LOGDIR

# output directory for evndisp products
if [[ -z $VERITAS_IRF_ANA_DIR ]]; then
    ODIR="$VERITAS_IRF_ANA_DIR/$EDVERSION/${PARTICLE_TYPE}_${EPOCH}_ATM${ATM}_${SIMTYPE}"
else
    ODIR="$VERITAS_DATA_DIR/analysis/$EDVERSION/${PARTICLE_TYPE}_${EPOCH}_ATM${ATM}_${SIMTYPE}"
fi
[[ $USEFROGS != 0 ]] && ODIR="${ODIR}_FROGS"
echo "Output files will be written to: $ODIR"
mkdir -p $ODIR

# Set number of files to loop over
if [[ $PARTICLE == "14" && $SIMTYPE = "GRISU" ]]; then
    NFILES=28
else
    NFILES=0
fi

# Create a unique set of run numbers
if [[ $SIMTYPE = "GRISU" ]]; then
    [[ $EPOCH == "V4" ]] && RUNNUM="946500"
    [[ $EPOCH == "V5" ]] && RUNNUM="956500"
    [[ $EPOCH == "V6" ]] && RUNNUM="966500"
elif [ $SIMTYPE = "CARE" ]; then
    [[ $EPOCH == "V4" ]] && RUNNUM="941200"
    [[ $EPOCH == "V5" ]] && RUNNUM="951200"
    [[ $EPOCH == "V6" ]] && RUNNUM="961200"
fi

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.evndisp_MC_sub"

for (( f = 0; f <= $NFILES; f++ )); do
    for ZA in ${ZENITH_ANGLES[@]}; do
        for WOBBLE in ${WOBBLE_OFFSETS[@]}; do
            for NOISE in ${NOISE_LEVELS[@]}; do
                echo "Now processing file $f, zenith angle $ZA, wobble $WOBBLE, noise level $NOISE"
                INT_WOBBLE=`echo "$WOBBLE*100" | bc | awk -F '.' '{print $1}'`
                
                # make run script
                FSCRIPT="$LOGDIR/evndisp_MC_$ZA-$WOBBLE-$NOISE-$EPOCH-ATM$ATM-$f"
                sed -e "s|FILENUMBER|$f|" \
                    -e "s|DATADIR|$SIMDIR|" \
                    -e "s|RUNNUMBER|$RUNNUM|" \
                    -e "s|ZENITHANGLE|$ZA|" \
                    -e "s|ATMOSPHERE|$ATM|" \
                    -e "s|OUTPUTDIR|$ODIR|" \
                    -e "s|DECIMALWOBBLE|$WOBBLE|" \
                    -e "s|INTEGERWOBBLE|$INT_WOBBLE|" \
                    -e "s|NOISELEVEL|$NOISE|" \
                    -e "s|ARRAYEPOCH|$EPOCH|" \
                    -e "s|FROGSFROGS|$USEFROGS|" \
                    -e "s|FROGSEVENTS|$NEVENTS|" \
                    -e "s|SIMULATIONTYPE|$SIMTYPE|" \
                    -e "s|PARTICLETYPE|$PARTICLE|" $SUBSCRIPT.sh > $FSCRIPT.sh

                chmod u+x $FSCRIPT.sh
                echo $FSCRIPT.sh

                # run locally or on cluster
                SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
                SUBC=`eval "echo \"$SUBC\""`
                if [[ $SUBC == *qsub* ]]; then
                    $SUBC $FSCRIPT.sh
                elif [[ $SUBC == *parallel* ]]; then
                    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
                fi
                
                # increment run number
                RUNNUM=$((RUNNUM+1))
            done
        done
    done
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
