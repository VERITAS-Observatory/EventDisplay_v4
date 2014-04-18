#!/bin/bash
# submit effective area analysis
# (output need to be combined afterwards)
# Author: Gernot Maier

# qsub parameters
h_cpu=11:29:00; h_vmem=8000M; tmpdir_size=10G

if [ $# -ne 5 ]; then
# begin help message
echo "
IRF generation: create partial effective area files from MC ROOT files
 (simulations that have been processed by both evndisp_MC and mscw_energy_MC)

IRF.generate_effective_area_parts.sh <cuts file> <epoch> <atmosphere> <Rec ID>
 <sim type>

required parameters:

    <cuts file>             gamma/hadron cuts file
        
    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
                            
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.
        
    <sim type>              original VBF file simulation type (e.g. GRISU, CARE)
    
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
CUTSFILE=$1
EPOCH=$2
ATM=$3
RECID=$4
SIMTYPE=$5

# Simulation-specific parameters
if [ $SIMTYPE = "GRISU" ]; then
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
    NOISE_LEVELS=( 075 100 150 200 250 325 425 550 750 1000 )
    WOBBLE_OFFSETS=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
    SIMID="1"
else
    # CARE simulation files
    ZENITH_ANGLES=( 00 20 30 35 )
    NOISE_LEVELS=( 50 80 120 170 230 290 370 450 )
    WOBBLE_OFFSETS=( 0.5 )
    SIMID="9"
fi

# Check that cuts file exists
CUTSFILE=${CUTSFILE%%.dat}
CUTS_NAME=`basename $CUTSFILE`
CUTS_NAME=${CUTS_NAME##ANASUM.GammaHadron.}
if [[ "$CUTSFILE" == `basename $CUTSFILE` ]]; then
    CUTSFILE="$VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles/$CUTSFILE.dat"
else
    CUTSFILE="$CUTSFILE.dat"
fi
if [ ! -f "$CUTSFILE" ]; then
    echo "Error, gamma/hadron cuts file not found, exiting..."
    exit 1
fi

# input directory containing mscw_energy_MC products
if [[ -z $VERITAS_IRF_ANA_DIR ]]; then
    INDIR="$VERITAS_IRF_ANA_DIR/$EDVERSION/gamma_${EPOCH}_ATM${ATM}_${SIMTYPE}/RecID${RECID}"
elif [[ ! -z $VERITAS_IRF_ANA_DIR || ! -d $INDIR ]]; then
    INDIR="$VERITAS_DATA_DIR/analysis/$EDVERSION/gamma_${EPOCH}_ATM${ATM}_${SIMTYPE}/RecID${RECID}"
elif [[ ! -d $INDIR ]]; then
    echo "Error, could not locate input directory. Locations searched:"
    echo "$VERITAS_IRF_ANA_DIR/$EDVERSION/gamma_${EPOCH}_ATM${ATM}_${SIMTYPE}/RecID${RECID}"
    echo "$VERITAS_DATA_DIR/analysis/$EDVERSION/gamma_${EPOCH}_ATM${ATM}_${SIMTYPE}/RecID${RECID}"
    exit 1
fi
echo "Input file directory: $INDIR"

# Output file directory
if [[ -z $VERITAS_IRF_ANA_DIR ]]; then
    ODIR="$VERITAS_IRF_ANA_DIR/$EDVERSION/EffectiveAreas/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}/$CUTS_NAME"
else
    ODIR="$VERITAS_DATA_DIR/analysis/$EDVERSION/EffectiveAreas/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}/$CUTS_NAME"
fi
echo "Output file directory: $ODIR"
mkdir -p $ODIR

# run scripts and output are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EFFAREA"
mkdir -p $LOGDIR

# copy cuts file to log directory
cp "$CUTSFILE" "$LOGDIR/$CUTSFILE"

#################################
# template string containing the name of processed simulation root file
TFILE='gamma_${ZA}deg_750m_w${WOBBLE}_ID${RECID}_ana${EPOCH}_NOISE${NLEVEL}_${SIMID}.root'

# parameter file template which will be filled inside the for loops
read -r -d '' PARAMFILE << 'PARAMFILECONTENTS'
* FILLINGMODE 0
* ENERGYRECONSTRUCTIONMETHOD 1
* ENERGYAXISBINS 60
* AZIMUTHBINS 1
* FILLMONTECARLOHISTOS 0
* ENERGYSPECTRUMINDEX 40 1.5 0.1
* FILLMONTECARLOHISTOS 0
* SHAPECUTINDEX 0
* CUTFILE $LOGDIR/$CUTSFILE
* SIMULATIONFILE_DATA $MCFILE
PARAMFILECONTENTS

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.effective_area_parallel_sub"

# loop over all zenith angles, wobble offsets, and noise bins
for ZA in ${ZENITH_ANGLES[@]}; do
    for NOISE in ${NOISE_LEVELS[@]}; do
        for WOBBLE in ${WOBBLE_OFFSETS[@]}; do
            echo "Processing Zenith = $ZA, Noise = $NOISE, Wobble = $WOBBLE"
            
            # generate MC file name from template; check if file exists
            MCFILE=$INDIR/`eval "echo \"$TFILE\""`
            if [ ! -e $MCFILE ]; then
                echo "Input file not found: $MCFILE"
                continue
            fi
            
            # create makeEffectiveArea parameter file
            EAPARAMS="$RECID-$ZA-$WOBBLE-$NOISE"
            rm -f "$LOGDIR/$EAPARAMS.dat"
            eval "echo \"$PARAMFILE\"" > $LOGDIR/$EAPARAMS.dat

            # set parameters in run script
            FSCRIPT="$LOGDIR/EA.$RECID.$DATE.MC"
            sed -e "s|OUTPUTDIR|$ODIR|" \
                -e "s|EAFILENAME|$EAPARAMS|" \
                -e "s|RUNPFILE|$LOGDIR/$EAPARAMS.dat|" $SUBSCRIPT.sh > $FSCRIPT.sh

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
        done
    done
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
