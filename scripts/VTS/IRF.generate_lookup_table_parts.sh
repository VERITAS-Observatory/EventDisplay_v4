#!/bin/bash
# script to run over all noise levels and create lookup tables (queue submit)
# Author: Gernot Maier

# qsub parameters
h_cpu=03:29:00; h_vmem=4000M; tmpdir_size=1G

if [ $# -ne 4 ]; then
# begin help message
echo "
IRF generation: create partial lookup tables from MC mscw_energy ROOT files

IRF.generate_lookup_table_parts.sh <epoch> <atmosphere> <Rec ID> <sim type>

required parameters:
        
    <epoch>                 array epoch (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
                            
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
    
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
EPOCH=$1
ATM=$2
RECID=$3
SIMTYPE=$4

# Simulation-specific parameters
if [[ $SIMTYPE = "GRISU" ]]; then
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
    SIM_NOISE=( 075 100 150 200 250 325 425 550 750 1000 )
    if [ $EPOCH = "V5" ] || [ $EPOCH = "V4" ]; then
        TABLE_NOISE=( 336 382 457 524 579 659 749 850 986 1138 )        # sumwindow = 6
        #TABLE_NOISE=( 375 430 515 585 650 740 840 950 1105 1280 )      # sumwindow = 7
        #TABLE_NOISE=( 535 610 730 840 935 1060 1210 1370 1595 1840 )   # sumwindow = 12
    else
        TABLE_NOISE=( 331 384 469 542 600 684 773 875 1002 1178 )       # sumwindow = 6
    fi
    WOBBLE_OFFSETS=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
elif [ $SIMTYPE = "CARE" ]; then
    # CARE simulation files
    ZENITH_ANGLES=( 00 20 30 35 )
    SIM_NOISE=( 50 80 120 170 230 290 370 450 )
    TABLE_NOISE=( 349 410 607 683 800 888 1001 1108 )
    WOBBLE_OFFSETS=( 0.5 )
else
    echo "Invalid simulation type. Exiting..."
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
    ODIR="$VERITAS_IRF_ANA_DIR/$EDVERSION/Tables/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}"
else
    ODIR="$VERITAS_DATA_DIR/analysis/$EDVERSION/Tables/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}"
fi
echo "Output file directory: $ODIR"
mkdir -p $ODIR

# run scripts and output are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.MAKETABLES"
mkdir -p $LOGDIR

SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.lookup_table_parallel_sub"

# loop over all zenith angles, wobble offsets, and noise bins
for ZA in ${ZENITH_ANGLES[@]}; do
    for WOBBLE in ${WOBBLE_OFFSETS[@]}; do
        for (( i=0; i < ${#SIM_NOISE[@]}; i++ )); do
            echo "Processing Zenith = $ZA, Wobble = $WOBBLE, Noise = ${SIM_NOISE[$i]}"
            
            FSCRIPT="$LOGDIR/$EPOCH-MK-TBL.$DATE.MC-$ZA-$WOBBLE-${SIM_NOISE[$i]}-$EPOCH-$ATM-$TELTOANA-$RECID"
            rm -f $FSCRIPT.sh

            sed -e "s|ZENITHANGLE|$ZA|" \
                -e "s|WOBBLEOFFSET|$WOBBLE|" \
                -e "s|SIMNOISE|${SIM_NOISE[$i]}|" \
                -e "s|TABLENOISE|${TABLE_NOISE[$i]}|" \
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
        done
    done
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
