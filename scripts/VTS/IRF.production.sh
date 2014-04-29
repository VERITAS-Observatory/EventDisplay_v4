#!/bin/bash
# IRF production script (VERITAS)
#

if [ $# -lt 2 ]; then
# begin help message
echo "
IRF generation: produce a full set of instrument response functions (IRFs)

IRF.production.sh <sim type> <IRF type> [epoch] [atmosphere] [Rec ID] [cuts list file] [sim directory]

required parameters:

    <sim type>              original VBF file simulation type (e.g. GRISU, CARE)
    
    <IRF type>              type of instrument response function to produce
                            (e.g. EVNDISP, MAKETABLES, ANALYSETABLES, EFFECTIVEAREA, RADIALACCEPTANCE)
    
optional parameters:
    
    [epoch]                 array epoch(s) (e.g., V4, V5, V6)
                            (default: \"V4 V5 V6\")
                            
    [atmosphere]            atmosphere model(s) (21 = winter, 22 = summer)
                            (default: \"21 22\")
                            
    [Rec ID]                reconstruction ID(s) (default: \"0 1 2 3 4\")
                            (see EVNDISP.reconstruction.runparameter)
    
    [cuts list file]        file containing one gamma/hadron cuts file per line
                            (default: hard-coded standard EventDisplay cuts)

    [sim directory]         directory containing simulation VBF files

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
SIMTYPE=$1
IRFTYPE=$2
[[ "$3" ]] && EPOCH=$3 || EPOCH="V4 V5 V6"
[[ "$4" ]] && ATMOS=$4 || ATMOS="21 22"
[[ "$5" ]] && RECID=$5 || RECID="0 1 2 3 4"
[[ "$6" ]] && CUTSLISTFILE=$6 || CUTSLISTFILE=""
[[ "$7" ]] && SIMDIR=$7 || SIMDIR=""

# simulation types and definition of parameter space
# (todo: remove some of the analysis derived values (e.g. pedvars)
if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
    # GrISU simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
    NSB_LEVELS=( 075 100 150 200 250 325 425 550 750 1000 )
    PEDVAR_LEVELS=( 336 382 457 524 579 659 749 850 986 1138 )        # sumwindow = 6 (needed for lookup table production only)
#   PEDVAR_LEVELS=( 375 430 515 585 650 740 840 950 1105 1280 )      # sumwindow = 7 (needed for lookup table production only)
#   PEDVAR_LEVELS=( 535 610 730 840 935 1060 1210 1370 1595 1840 )   # sumwindow = 12 (needed for lookup table production only)
    WOBBLE_OFFSETS=( 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
    WOBBLE_OFFSETS=( 0.50 )
    TABLEFILE="table_v441rc_d20140429_GrIsuDec12_ATM21_VX_ID0.root"
elif [ ${SIMTYPE:0:4} = "CARE" ]; then
    # CARE simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 )
    NSB_LEVELS=( 50 80 120 170 230 290 370 450 )
    PEDVAR_LEVELS=( 349 410 607 683 800 888 1001 1108 )  # sumwindow = 6 (needed for lookup table production only)
    WOBBLE_OFFSETS=( 0.5 )
else
    echo "Invalid simulation type. Exiting..."
    exit 1
fi


# Set gamma/hadron cuts
if [[ $CUTSLISTFILE != "" ]]; then
    if [ ! -f $CUTSLISTFILE ]; then
        echo "Error, cuts list file not found, exiting..."
        exit 1
    fi
    # read file containing list of cuts
    IFS=$'\r\n' CUTLIST=($(cat $CUTLISTFILE))
else
    # default hard-coded cuts
    CUTLIST="ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open
             ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-SuperSoft
             ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft
             ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate"
fi


for VX in $EPOCH; do
    for ATM in $ATMOS; do
        for ZA in ${ZENITH_ANGLES[@]}; do
            for WOBBLE in ${WOBBLE_OFFSETS[@]}; do
                NNOISE=${#NSB_LEVELS[@]}
                for (( i = 0 ; i < $NNOISE; i++ )); do
                    echo "Now processing epoch $VX, atmo $ATM, zenith angle $ZA, wobble $WOBBLE, noise level ${NSB_LEVELS[$i]}"
                    # run simulations through evndisp
                    if [[ $IRFTYPE == "EVNDISP" ]]; then
                        if [[ -z $SIMDIR ]]; then
                           if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
                              SIMDIR=$VERITAS_DATA_DIR/simulations/"$VX"_FLWO/grisu/ATM"$ATM"
                           elif [[ ${SIMTYPE:0:4} = "CARE" ]]; then
                              SIMDIR=$VERITAS_DATA_DIR/simulations/"$VX"_FLWO/${SIMTYPE}
                           fi
                        fi
                        ./IRF.evndisp_MC.sh $SIMDIR $VX $ATM $ZA $WOBBLE ${NSB_LEVELS[$i]} $SIMTYPE
                    # make tables
                    elif [[ $IRFTYPE == "MAKETABLES" ]]; then
                       ./IRF.generate_lookup_table_parts.sh $VX $ATM $ZA $WOBBLE ${NSB_LEVELS[$i]} ${PEDVAR_LEVELS[$i]} 0 $SIMTYPE
                    # analyse table files
                    elif [[ $IRFTYPE == "ANALYSETABLES" ]]; then
                        TFIL="${TABLEFILE/VX/$VX}"
                        for ID in $RECID; do
                           ./IRF.mscw_energy_MC.sh $TFIL $VX $ATM $ZA $WOBBLE ${NSB_LEVELS[$i]} $ID $SIMTYPE
                        done
                    fi

#### TODO: add effective areas ####

#            for CUTS in ${CUTLIST[@]}; do
#                # analyse effective areas
#                if [[ $IRFTYPE == "EFFECTIVEAREAS" ]]; then
#                   echo "effective areas"
#                fi
                done
            done
        done
    done
done

exit
