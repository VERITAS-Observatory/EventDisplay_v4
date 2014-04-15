#!/bin/bash
# submit evndisp for simulations (CARE)
# Author: Gernot Maier 

# qsub parameters
h_cpu=48:49:00; h_vmem=250M; tmpdir_size=4G

if [ $# -lt 4 ]; then
# begin help message
echo "
IRF generation: analyze CARE simulation VBF files using evndisp 

IRF.evndisp_MC_CARE.sh <sim directory> <output directory> <epoch> <atmosphere>
 [particle]

required parameters:

    <sim directory>         directory containing simulation VBF files
    
    <output directory>      directory which will store output ROOT files

    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)

optional parameters:
    
    [particle]              type of particle used in simulation:
                            gamma = 1, electron = 2, proton = 14, helium = 402
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
SIMDIR=$1
ODIR=$2
EPOCH=$3
ATM=$4
[[ "$5" ]] && PARTICLE=$5 || PARTICLE=1

# CARE simulation parameters
ZENITH_ANGLES=( 00 20 30 35 )
NOISE_LEVELS=( 50 80 120 170 230 290 370 450 )
WOBBLE_OFFSETS=( 0.5 )

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EVNDISP.ANAMCVBF"
mkdir -p $LOGDIR

# output directory for anasum products
mkdir -p $ODIR

# Create a unique set of run numbers
[[ $EPOCH == "V4" ]] && RUNNUM="941200"
[[ $EPOCH == "V5" ]] && RUNNUM="951200"
[[ $EPOCH == "V6" ]] && RUNNUM="961200"

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.evndisp_MC_CARE_sub.sh"

for ZA in ${ZENITH_ANGLES[@]}; do
    for WOBBLE in ${WOBBLE_OFFSETS[@]}; do
        for NOISE in ${NOISE_LEVELS[@]}; do
            echo "Now processing zenith angle $ZA, wobble $WOBBLE, noise level $NOISE"

            # make run script
            FSCRIPT="$LOGDIR/$EPOCH-EV_CARE_VBF-$ZA-$WOBBLE-$NOISE-$EPOCH-ATM$ATM"
            sed -e "s|ZENITHANGLE|$ZA|" \
                -e "s|DATADIR|$SIMDIR|" \
                -e "s|ATMOSPHERE|$ATM|" \
                -e "s|OUTPUTDIR|$ODIR|" \
                -e "s|WOBBLEOFF|$WOBBLE|" \
                -e "s|NOISELEVEL|$NOISE|" \
                -e "s|ARRAYEPOCH|$EPOCH|" \
                -e "s|RUNNUMBER|$RUNNUM|" \
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
            (( RUNNUM++ ))
        done
    done
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
