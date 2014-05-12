#!/bin/bash
# IRF production script (VERITAS)
#

if [[ $# < 2 ]]; then
# begin help message
echo "
IRF generation: produce a full set of instrument response functions (IRFs)

IRF.production.sh <sim type> <IRF type> [epoch] [atmosphere] [Rec ID]
 [cuts list file] [sim directory]

required parameters:

    <sim type>              original VBF file simulation type (e.g. GRISU, CARE)
    
    <IRF type>              type of instrument response function to produce
                            (e.g. EVNDISP, ANALYSE_TABLES, MAKE_TABLES,
                                  EFFECTIVE_AREA, COMBINE_EFFECTIVE_AREAS)
    
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
    WOBBLE_OFFSETS=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
    TABLEFILE="table_v441rc_d20140503_GrIsuDec12_ATM21_VX_ID0"
elif [ ${SIMTYPE:0:4} = "CARE" ]; then
    # CARE simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
    NSB_LEVELS=( 50 80 120 170 230 290 370 450 )
    WOBBLE_OFFSETS=( 0.5 )
    TABLEFILE="table_v441rc_d20140503_CARE_Jan1427_ATM21_VX_ID0"
else
    echo "Invalid simulation type. Exiting..."
    exit 1
fi

# Starting run numbers for evndisp analysis
if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
    RUNNUM="970000"
elif [ ${SIMTYPE:0:4} = "CARE" ]; then
    RUNNUM="980000"
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
    # default list of cuts
    CUTLIST="ANASUM.GammaHadron-Cut-NTel2-PointSource-SuperSoftSpectrum.dat 
             ANASUM.GammaHadron-Cut-NTel3-PointSource-ModerateSpectrum.dat 
             ANASUM.GammaHadron-Cut-NTel3-PointSource-SoftSpectrum.dat 
             ANASUM.GammaHadron-Cut-NTel3-PointSource-HardSpectrum.dat 
             ANASUM.GammaHadron-Cut-NTel2-PointSource-Open.dat" 
#    CUTLIST="ANASUM.GammaHadron-Cut-NTel2-PointSource-SuperSoftSpectrum.dat "
fi

# loop over complete parameter space and submit production
for VX in $EPOCH; do
    for ATM in $ATMOS; do
       # combine effective areas
       if [[ $IRFTYPE == "COMBINEEFFECTIVEAREAS" ]]; then
            for ID in $RECID; do
                for CUTS in ${CUTLIST[@]}; do
                    echo "combine effective areas $CUTS"
                   ./IRF.combine_effective_area_parts.sh $CUTS $VX $ATM $ID $SIMTYPE
                done # cuts
            done
            continue
        fi
        for ZA in ${ZENITH_ANGLES[@]}; do
            for WOBBLE in ${WOBBLE_OFFSETS[@]}; do
                for NOISE in ${NSB_LEVELS[@]}; do
                    echo "Now processing epoch $VX, atmo $ATM, zenith angle $ZA, wobble $WOBBLE, noise level $NOISE"
                    # run simulations through evndisp
                    if [[ $IRFTYPE == "EVNDISP" ]]; then
                        if [[ -z $SIMDIR ]]; then
                           if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
                              SIMDIR="$VERITAS_DATA_DIR/simulations/${VX}_FLWO/grisu/ATM$ATM"
                           elif [[ ${SIMTYPE:0:4} = "CARE" ]]; then
                              SIMDIR="$VERITAS_DATA_DIR/simulations/${VX}_FLWO/$SIMTYPE"
                           fi
                        fi
                        ./IRF.evndisp_MC.sh $RUNNUM $SIMDIR $VX $ATM $ZA $WOBBLE $NOISE $SIMTYPE
                        ((RUNNUM++))
                    # make tables
                    elif [[ $IRFTYPE == "MAKETABLES" ]]; then
                       ./IRF.generate_lookup_table_parts.sh $VX $ATM $ZA $WOBBLE $NOISE 0 $SIMTYPE
                    # analyse table files
                    elif [[ $IRFTYPE == "ANALYSETABLES" ]]; then
                        TFIL="${TABLEFILE/VX/$VX}"
                        for ID in $RECID; do
                           ./IRF.mscw_energy_MC.sh $TFIL $VX $ATM $ZA $WOBBLE $NOISE $ID $SIMTYPE
                        done
                    # analyse effective areas
                    elif [[ $IRFTYPE == "EFFECTIVEAREAS" ]]; then
                        for ID in $RECID; do
                            for CUTS in ${CUTLIST[@]}; do
                                echo "effective areas $CUTS"
                               ./IRF.generate_effective_area_parts.sh $CUTS $VX $ATM $ZA $WOBBLE $NOISE $ID $SIMTYPE
                            done #cuts
                        done #recID
                    fi
                done #noise
            done #wobble
        done #ZA
    done #ATM
done  #VX

exit
