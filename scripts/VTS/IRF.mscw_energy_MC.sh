#!/bin/bash
# script to analyse MC files with lookup tables

# qsub parameters
h_cpu=10:29:00; h_vmem=6000M; tmpdir_size=100G

if [[ $# < 8 ]]; then
# begin help message
echo "
IRF generation: analyze simulation evndisp ROOT files using mscw_energy 

IRF.mscw_energy_MC.sh <table file> <epoch> <atmosphere> <zenith> <offset angle> <NSB level> <Rec ID> <sim type> [particle]

required parameters:

    <table file>            mscw_energy lookup table file
    
    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)

    <zenith>                zenith angle of simulations [deg]

    <offset angle>          offset angle of simulations [deg]

    <NSB level>             NSB level of simulations [MHz]
    
    <Rec ID>                reconstruction ID (can be a list of recids)
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.

    <sim type>              simulation type (e.g. GRISU-SW6, CARE_June1425)

optional parameters:
    
    [particle]              type of particle used in simulation:
                            gamma = 1, proton = 14, alpha (helium) = 402
                            (default = 1  -->  gamma)
                            
--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# EventDisplay version
IRFVERSION=`$EVNDISPSYS/bin/mscw_energy --version | tr -d .| sed -e 's/[a-Z]*$//'`

# Parse command line arguments
TABFILE=$1
TABFILE=${TABFILE%%.root}.root
EPOCH=$2
ATM=$3
ZA=$4
WOBBLE=$5
NOISE=$6
RECID=$7
SIMTYPE=$8
[[ "$9" ]] && PARTICLE=$9 || PARTICLE=1

# Particle names
PARTICLE_NAMES=( [1]=gamma [2]=electron [14]=proton [402]=alpha )
PARTICLE_TYPE=${PARTICLE_NAMES[$PARTICLE]}

# Check that table file exists
if [[ "$TABFILE" == `basename $TABFILE` ]]; then
    TABFILE="$VERITAS_EVNDISP_AUX_DIR/Tables/$TABFILE"
fi
if [[ ! -f "$TABFILE" ]]; then
    echo "Error, table file $TABFILE not found, exiting..."
    exit 1
fi

# Hack to scale sizes based on epochs to approx. correct for drop in reflectivity+gain.
_sizecallineraw=$(grep "* s " ${VERITAS_EVNDISP_AUX_DIR}/ParameterFiles/MSCW.sizecal.runparameter | grep " ${EPOCH} ")
EPOCH_LABEL=$(echo "$_sizecallineraw" | awk '{print $3}')
EPOCH_T1SCALE=$(echo "$_sizecallineraw" | awk '{print $4}')
EPOCH_T2SCALE=$(echo "$_sizecallineraw" | awk '{print $5}')
EPOCH_T3SCALE=$(echo "$_sizecallineraw" | awk '{print $6}')
EPOCH_T4SCALE=$(echo "$_sizecallineraw" | awk '{print $7}')

if ! [ $(echo ${EPOCH_T1SCALE} | awk '$1>0.0 && $1<2.0 {print 1}')==1 ]; then
    echo "T1 SCALING ${EPOCH_T1SCALE} is invalid"
    exit 1
fi
if ! [ $(echo ${EPOCH_T2SCALE} | awk '$1>0.0 && $1<2.0 {print 1}')==1 ]; then
    echo "T2 SCALING ${EPOCH_T2SCALE} is invalid"
    exit 1
fi
if ! [ $(echo ${EPOCH_T3SCALE} | awk '$1>0.0 && $1<2.0 {print 1}')==1 ]; then
    echo "T3 SCALING ${EPOCH_T3SCALE} is invalid"
    exit 1
fi
if ! [ $(echo ${EPOCH_T4SCALE} | awk '$1>0.0 && $1<2.0 {print 1}')==1 ]; then
    echo "T4 SCALING ${EPOCH_T4SCALE} is invalid"
    exit 1
fi

SIZESCALING="$EPOCH_T1SCALE,$EPOCH_T2SCALE,$EPOCH_T3SCALE,$EPOCH_T4SCALE"

# input directory containing evndisp products
if [[ -n "$VERITAS_IRFPRODUCTION_DIR" ]]; then
    INDIR="$VERITAS_IRFPRODUCTION_DIR/$IRFVERSION/$SIMTYPE/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}/ze${ZA}deg_offset${WOBBLE}deg_NSB${NOISE}MHz"
fi
if [[ ! -d $INDIR ]]; then
    echo -e "Error, could not locate input directory. Locations searched:\n $INDIR"
    exit 1
fi
echo "Input file directory: $INDIR"

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.ANATABLES/$(date +%s | cut -c -8)/"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# Output file directory
if [[ -n "$VERITAS_IRFPRODUCTION_DIR" ]]; then
    ODIR="$VERITAS_IRFPRODUCTION_DIR/$IRFVERSION/$SIMTYPE/${EPOCH_LABEL}_ATM${ATM}_${PARTICLE_TYPE}"
fi
echo -e "Output files will be written to:\n $ODIR"

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.mscw_energy_MC_sub"

echo "Now processing zenith angle $ZA, wobble $WOBBLE, noise level $NOISE"

# make run script
FSCRIPT="$LOGDIR/MSCW-$EPOCH-$ZA-$WOBBLE-$NOISE-$PARTICLE"
sed -e "s|INPUTDIR|$INDIR|" \
    -e "s|OUTPUTDIR|$ODIR|" \
    -e "s|TABLEFILE|$TABFILE|" \
    -e "s|ZENITHANGLE|$ZA|" \
    -e "s|NOISELEVEL|$NOISE|" \
    -e "s|WOBBLEOFFSET|$WOBBLE|" \
    -e "s|SIZESCALING|$SIZESCALING|" \
    -e "s|RECONSTRUCTIONID|$RECID|" $SUBSCRIPT.sh > $FSCRIPT.sh

chmod u+x $FSCRIPT.sh
echo "Run script written to: $FSCRIPT"

# run locally or on cluster
SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
SUBC=`eval "echo \"$SUBC\""`
if [[ $SUBC == *qsub* ]]; then
    JOBID=`$SUBC $FSCRIPT.sh`
    echo "JOBID: $JOBID"	  
elif [[ $SUBC == *parallel* ]]; then
    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
fi

exit
