#!/bin/bash
# submit evndisp for grisu/care simulations
#

# qsub parameters
h_cpu=47:59:00; h_vmem=6000M; tmpdir_size=250G

if [ $# -lt 7 ]; then
# begin help message
echo "
IRF generation: analyze simulation VBF files using evndisp 

IRF.evndisp_MC.sh <sim directory> <epoch> <atmosphere> <zenith> <offset angle> <NSB level> <sim type> [particle] [Model3D] [FROGS] [events] [Rec ID]

required parameters:

    <sim directory>         directory containing simulation VBF files

    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)

    <zenith>                zenith angle of simulations [deg]

    <offset angle>          offset angle of simulations [deg]

    <NSB level>             NSB level of simulations [MHz]
    
    <sim type>              file simulation type (e.g. GRISU-SW6, CARE_June1425)
                            (recognized are also types like GRISU_d201404, or CARE_V1)

optional parameters:
    
    [particle]              type of particle used in simulation:
                            gamma = 1, electron = 2, proton = 14, helium = 402
                            (default = 1  -->  gamma)

    [Model3D]               set to 1 to use Model3D (default: off)
    
    [FROGS]                 set to 1 to use FROGS (GrISU only! default: off)
    
    [events]                FROGS ONLY: number of events per division
                            (default: 5000000)
   
    [Rec ID]                FROGS ONLY: reconstruction ID used at the MSCW stage 
                         
Note: zenith angles, wobble offsets, and noise values are hard-coded into script

GrISU simulation parameters
    ZENITH_ANGLES  = ( 00 20 30 35 40 45 50 55 60 65 )
    NSB_LEVELS     = ( 075 100 150 200 250 325 425 550 750 1000 )
    WOBBLE_OFFSETS = ( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )

CARE simulation parameters
    ZENITH_ANGLES = ( 00 20 30 35 40 45 50 55 60 65 )
    NSB_LEVELS    = ( 50 80 120 170 230 290 370 450 )
--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# EventDisplay version
EDVERSION=`$EVNDISPSYS/bin/evndisp --version | tr -d .`

# Parse command line arguments
SIMDIR=$1
EPOCH=$2
ATM=$3
ZA=$4
WOBBLE=$5
NOISE=$6
SIMTYPE=$7
[[ "$8" ]] && PARTICLE=$8 || PARTICLE=1
[[ "$9" ]] && USEMODEL3D=$9 || USEMODEL3D=0
[[ "${10}" ]] && USEFROGS=${10} || USEFROGS=0
[[ "${11}" ]] && NEVENTS=${11}  || NEVENTS=5000000
[[ "${12}" ]] && RECID=${12}  || RECID=-1

# Particle names
PARTICLE_NAMES=( [1]=gamma [2]=electron [14]=proton [402]=alpha )
PARTICLE_TYPE=${PARTICLE_NAMES[$PARTICLE]}

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EVNDISP.ANAMCVBF"
mkdir -p $LOGDIR

# output directory for evndisp products (will be manipulated more later in the script)
if [[ ! -z "$VERITAS_IRFPRODUCTION_DIR" ]]; then
    ODIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/${SIMTYPE}/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}"
fi
# output dir
OPDIR=$ODIR"/ze"$ZA"deg_offset"$WOBBLE"deg_NSB"$NOISE"MHz"
mkdir -p $OPDIR
echo -e "Output files will be written to:\n $OPDIR"

[[ $USEFROGS != 0 ]] && ODIR="${ODIR}_FROGS" && MSCWDIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}/MSCW_RECID$RECID"

# Create a unique set of run numbers
if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
    [[ $EPOCH == "V4" ]] && RUNNUM="946500"
    [[ $EPOCH == "V5" ]] && RUNNUM="956500"
    [[ $EPOCH == "V6" ]] && RUNNUM="966500"
elif [ ${SIMTYPE:0:4} = "CARE" ]; then
    [[ $EPOCH == "V4" ]] && RUNNUM="941200"
    [[ $EPOCH == "V5" ]] && RUNNUM="951200"
    [[ $EPOCH == "V6" ]] && RUNNUM="961200"
fi

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.evndisp_frogs_MC_sub"

INT_WOBBLE=`echo "$WOBBLE*100" | bc | awk -F '.' '{print $1}'`
if [[ ${#INT_WOBBLE} -lt 2 ]]; then
   INT_WOBBLE="000"
elif [[ ${#INT_WOBBLE} -lt 3 ]]; then
   INT_WOBBLE="0$INT_WOBBLE"
fi

# make run script
FSCRIPT="$LOGDIR/evn-$EPOCH-$SIMTYPE-$ZA-$WOBBLE-$NOISE-ATM$ATM"
sed -e "s|DATADIR|$SIMDIR|" \
    -e "s|RUNNUMBER|$RUNNUM|" \
    -e "s|ZENITHANGLE|$ZA|" \
    -e "s|ATMOSPHERE|$ATM|" \
    -e "s|OUTPUTDIR|$OPDIR|" \
    -e "s|DECIMALWOBBLE|$WOBBLE|" \
    -e "s|INTEGERWOBBLE|$INT_WOBBLE|" \
    -e "s|NOISELEVEL|$NOISE|" \
    -e "s|ARRAYEPOCH|$EPOCH|" \
    -e "s|USEMODEL3DMETHOD|$USEMODEL3D|" \
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
    ###JOBID=`$SUBC $FSCRIPT.sh`
	 JOBID=`$SUBC -t 1-10 $FSCRIPT.sh`
    echo "RUN $RUNNUM: JOBID $JOBID"
elif [[ $SUBC == *parallel* ]]; then
    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
fi
                
exit