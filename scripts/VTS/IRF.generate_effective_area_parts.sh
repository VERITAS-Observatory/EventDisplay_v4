#!/bin/bash
# submit effective area analysis
# (output need to be combined afterwards)

# qsub parameters
h_cpu=11:29:00; h_vmem=8000M; tmpdir_size=10G

if [[ $# != 8 ]]; then
# begin help message
echo "
IRF generation: create partial effective area files from MC ROOT files
 (simulations that have been processed by both evndisp_MC and mscw_energy_MC)

IRF.generate_effective_area_parts.sh <cuts file> <epoch> <atmosphere> <zenith>
 <offset angle> <NSB level> <Rec ID> <sim type>

required parameters:

    <cuts file>             gamma/hadron cuts file (located in 
                             \$VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles)
        
    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
                            
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)

    <zenith>                zenith angle of simulations [deg]

    <offset angle>          offset angle of simulations [deg]

    <NSB level>             NSB level of simulations [MHz]
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.
        
    <sim type>              simulation type (e.g. GRISU, CARE)
    
--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# EventDisplay version
EDVERSION=`$EVNDISPSYS/bin/makeEffectiveArea --version | tr -d .`

# Parse command line arguments
CUTSFILE=$1
EPOCH=$2
ATM=$3
ZA=$4
WOBBLE=$5
NOISE=$6
RECID=$7
SIMTYPE=$8
PARTICLE_TYPE="gamma"

# Check that cuts file exists
CUTSFILE=${CUTSFILE%%.dat}
CUTS_NAME=`basename $CUTSFILE`
CUTS_NAME=${CUTS_NAME##ANASUM.GammaHadron-}
if [[ "$CUTSFILE" == `basename $CUTSFILE` ]]; then
    CUTSFILE="$VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles/$CUTSFILE.dat"
else
    CUTSFILE="$CUTSFILE.dat"
fi
if [[ ! -f "$CUTSFILE" ]]; then
    echo "Error, gamma/hadron cuts file not found, exiting..."
    exit 1
fi

# input directory containing mscw_energy_MC products
if [[ -n $VERITAS_IRFPRODUCTION_DIR ]]; then
    INDIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}/RecID${RECID}"
elif [[ -z $VERITAS_IRFPRODUCTION_DIR || ! -d $INDIR ]]; then
    INDIR="$VERITAS_USER_DATA_DIR/analysis/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}/RecID${RECID}"
elif [[ ! -d $INDIR ]]; then
    echo -e "Error, could not locate input directory. Locations searched:\n $INDIR"
    exit 1
fi
echo "Input file directory: $INDIR"

# Output file directory
if [[ -n "$VERITAS_IRFPRODUCTION_DIR" ]]; then
    ODIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}/EffectiveAreas_${CUTS_NAME}"
else
    ODIR="$VERITAS_USER_DATA_DIR/analysis/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}/EffectiveAreas_${CUTS_NAME}"
fi
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR

# run scripts and output are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EFFAREA/${EPOCH}_ATM${ATM}_${PARTICLE_TYPE}/"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# copy cuts file to log directory
cp "$CUTSFILE" "$LOGDIR/"
CUTSFILE=`basename $CUTSFILE`

#################################
# template string containing the name of processed simulation root file
MCFILE="${INDIR}/${ZA}deg_${WOBBLE}wob_NOISE${NOISE}.mscw.root"
if [[ ! -f ${MCFILE} ]]; then
    echo "Input mscw file not found: ${MCFILE}"
    exit 1
fi

# parameter file template
PARAMFILE="
* FILLINGMODE 0
* ENERGYRECONSTRUCTIONMETHOD 1
* ENERGYAXISBINS 60
* AZIMUTHBINS 1
* FILLMONTECARLOHISTOS 0
* ENERGYSPECTRUMINDEX 40 1.5 0.1
* FILLMONTECARLOHISTOS 0
* CUTFILE $LOGDIR/$CUTSFILE
* SIMULATIONFILE_DATA $MCFILE"

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.effective_area_parallel_sub"

echo "Processing Zenith = $ZA, Noise = $NOISE, Wobble = $WOBBLE"
            
# create makeEffectiveArea parameter file
EAPARAMS="EffArea-${SIMTYPE}-${EPOCH}-ID${RECID}-Ze${ZA}deg-${WOBBLE}wob-${NOISE}-${CUTS_NAME}"
rm -f "$LOGDIR/$EAPARAMS.dat"
eval "echo \"$PARAMFILE\"" > $LOGDIR/$EAPARAMS.dat

# set parameters in run script
FSCRIPT="$LOGDIR/EA.ID${RECID}.$DATE.${CUTS_NAME}.MC"
sed -e "s|OUTPUTDIR|$ODIR|" \
    -e "s|EAFILENAME|$EAPARAMS|" \
    -e "s|RUNPFILE|$LOGDIR/$EAPARAMS.dat|" $SUBSCRIPT.sh > $FSCRIPT.sh

chmod u+x $FSCRIPT.sh
echo $FSCRIPT.sh

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
