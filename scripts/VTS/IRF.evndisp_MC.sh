#!/bin/bash
# submit evndisp for grisu/care simulations (analyse all noise levels at the same time)

# qsub parameters
h_cpu=47:59:00; h_vmem=6000M; tmpdir_size=250G

if [[ $# < 8 ]]; then
# begin help message
echo "
IRF generation: analyze simulation VBF files using evndisp 

IRF.evndisp_MC.sh <run number> <sim directory> <epoch> <atmosphere> <zenith>
 <offset angle> <NSB level> <sim type> [particle] [FROGS] [mscw dir] [events]

required parameters:

    <run number>            artificial run number that will be used by evndisp
                            to keep track of each sim file and param set

    <sim file>              full path (including directory) of MC VBF file

    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)

    <zenith>                zenith angle of simulations [deg]

    <offset angle>          offset angle of simulations [deg]

    <NSB level>             NSB level of simulations [MHz]
    
    <sim type>              original VBF file simulation type (e.g. GRISU, CARE,
                            GRISU_d201404, CARE_V1, etc.)

optional parameters:
    
    [particle]              type of particle used in simulation:
                            gamma = 1, electron = 2, proton = 14, helium = 402
                            (default = 1  -->  gamma)
    
    [FROGS]                 set to 1 to use FROGS (GrISU only! default: off)
    
    [mscw directory]        directory which contains mscw_energy files
                            (FROGS only; default = blank)
    
    [events]                FROGS ONLY: number of events per division
                            (default: 5000000)
                            
Note: zenith angles, wobble offsets, and noise values are hard-coded into script

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
SIMDIR=$2
EPOCH=$3
ATM=$4
ZA=$5
WOBBLE=$6
NOISE=$7
SIMTYPE=$8
[[ "$9" ]] && PARTICLE=$9 || PARTICLE=1
[[ "${10}" ]] && USEFROGS=${10} || USEFROGS=0
[[ "${11}" ]] && MSCWDIR=${11}
[[ "${12}" ]] && NEVENTS=${12}  || NEVENTS=5000000

# Particle names
PARTICLE_NAMES=( [1]=gamma [2]=electron [14]=proton [402]=alpha )
PARTICLE_TYPE=${PARTICLE_NAMES[$PARTICLE]}

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EVNDISP.ANAMCVBF"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# output directory for evndisp products
if [[ -n "$VERITAS_IRFPRODUCTION_DIR" ]]; then
    ODIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}"
else
    ODIR="$VERITAS_USER_DATA_DIR/analysis/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}"
fi
[[ $USEFROGS != 0 ]] && ODIR="${ODIR}_FROGS"
ODIR="$ODIR/ze${ZA}deg_offset${WOBBLE}deg_NSB${NOISE}MHz"
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR


# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.evndisp_MC_sub"

INT_WOBBLE=`echo "$WOBBLE*100" | bc | awk -F '.' '{print $1}'`
if [[ ${#INT_WOBBLE} < 2 ]]; then
   INT_WOBBLE="000"
elif [[ ${#INT_WOBBLE} < 3 ]]; then
   INT_WOBBLE="0$INT_WOBBLE"
fi

# make run script
FSCRIPT="$LOGDIR/evn-$SIMTYPE-$ZA-$WOBBLE-$NOISE-$EPOCH-ATM$ATM"
sed -e "s|RUNNUMBER|$RUNNUM|" \
    -e "s|MCVBFDIR|$SIMDIR|" \
    -e "s|ZENITHANGLE|$ZA|" \
    -e "s|ATMOSPHERE|$ATM|" \
    -e "s|OUTPUTDIR|$ODIR|" \
    -e "s|DECIMALWOBBLE|$WOBBLE|" \
    -e "s|INTEGERWOBBLE|$INT_WOBBLE|" \
    -e "s|NOISELEVEL|$NOISE|" \
    -e "s|ARRAYEPOCH|$EPOCH|" \
    -e "s|FROGSFROGS|$USEFROGS|" \
    -e "s|FROGSMSCWDIR|$MSCWDIR|" \
    -e "s|FROGSEVENTS|$NEVENTS|" \
    -e "s|SIMULATIONTYPE|$SIMTYPE|" \
    -e "s|PARTICLETYPE|$PARTICLE|" $SUBSCRIPT.sh > $FSCRIPT.sh

chmod u+x $FSCRIPT.sh
echo $FSCRIPT.sh

# run locally or on cluster
SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
SUBC=`eval "echo \"$SUBC\""`
if [[ $SUBC == *qsub* ]]; then
    JOBID=`$SUBC $FSCRIPT.sh`
    echo "RUN $RUNNUM: JOBID $JOBID"
elif [[ $SUBC == *parallel* ]]; then
    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
fi

exit
