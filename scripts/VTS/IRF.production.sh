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
                            (e.g. EVNDISP, MAKETABLES, COMBINETABLES,
                             ANALYSETABLES, EFFECTIVEAREAS, COMBINEEFFECTIVEAREAS,
                             MVAEVNDISP, TRAINMVANGRES )
    
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
# evndisplay version
EDVERSION=`$EVNDISPSYS/bin/mscw_energy --version | tr -d .`
# version string for aux files
AUX="auxv01"

# simulation types and definition of parameter space
if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
    # GrISU simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
    NSB_LEVELS=( 075 100 150 200 250 325 425 550 750 1000 )
    WOBBLE_OFFSETS=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
    if [[ $IRFTYPE == "MVAEVNDISP" ]]; then
       NSB_LEVELS=( 200 )
       WOBBLE_OFFSETS=( 0.5 )
    fi
elif [ ${SIMTYPE:0:4} = "CARE" ]; then
    # CARE simulation parameters
    ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 60 65 )
    NSB_LEVELS=( 50 80 120 170 230 290 370 450 )
    WOBBLE_OFFSETS=( 0.5 )
    if [[ $IRFTYPE == "MVAEVNDISP" ]]; then
       NSB_LEVELS=( 170 )
       WOBBLE_OFFSETS=( 0.5 )
    fi
else
    echo "Invalid simulation type. Exiting..."
    exit 1
fi
# table file name
# (REC ID is set later)
TABLEFILE="table-${EDVERSION}-${AUX}-${SIMTYPE}-ATM${ATMOS}-${EPOCH}-ID"

# combined table file name
# (METHOD "DISP" or "GEO" is set later)
TABLECOM="table-${EDVERSION}-${AUX}-${SIMTYPE}-ATM${ATMOS}-${EPOCH}-"

# Set gamma/hadron cuts
if [[ $CUTSLISTFILE != "" ]]; then
    if [ ! -f $CUTSLISTFILE ]; then
        echo "Error, cuts list file not found, exiting..."
        exit 1
    fi
    # read file containing list of cuts
    IFS=$'\r\n' CUTLIST=($(cat $CUTSLISTFILE))
else
    # default list of cuts
    CUTLIST="ANASUM.GammaHadron-Cut-NTel2-PointSource-Moderate.dat 
             ANASUM.GammaHadron-Cut-NTel2-PointSource-Soft.dat 
             ANASUM.GammaHadron-Cut-NTel2-PointSource-Hard.dat 
             ANASUM.GammaHadron-Cut-NTel3-PointSource-SuperHard.dat 
             ANASUM.GammaHadron-Cut-NTel2-PointSource-ModerateOpen.dat
             ANASUM.GammaHadron-Cut-NTel2-PointSource-SoftOpen.dat 
             ANASUM.GammaHadron-Cut-NTel2-PointSource-HardOpen.dat 
             ANASUM.GammaHadron-Cut-NTel2-ExtendedSource-Moderate.dat 
             ANASUM.GammaHadron-Cut-NTel2-ExtendedSource-Soft.dat 
             ANASUM.GammaHadron-Cut-NTel2-ExtendedSource-Hard.dat 
             ANASUM.GammaHadron-Cut-NTel3-PointSource-Moderate.dat 
             ANASUM.GammaHadron-Cut-NTel3-PointSource-Hard.dat
             ANASUM.GammaHadron-Cut-NTel3-ExtendedSource-SuperHard.dat"
#     CUTLIST="ANASUM.GammaHadron-Cut-NTel2-PointSource-Moderate.dat"
fi

############################################################
# loop over complete parameter space and submit production
for VX in $EPOCH; do
    for ATM in $ATMOS; do
       ######################
       # combine lookup tables
       if [[ $IRFTYPE == "COMBINETABLES" ]]; then
            TFIL="${TABLECOM}"
            for ID in $RECID; do
                echo "combine lookup tables"
                if [[ $ID == "0" ]] || [[ $ID == "2" ]] || [[ $ID == "3" ]] || [[ $ID == "4" ]] || [[ $ID == "5" ]] || [[ $ID == "6" ]]; then
		            METH="GEO"
                elif [[ $ID == "1" ]] || [[ $ID == "7" ]] || [[ $ID == "8" ]] || [[ $ID == "9" ]] || [[ $ID == "10" ]]; then 
		            METH="DISP"
		        fi
                ./IRF.combine_lookup_table_parts.sh "${TFIL}${METH}" $VX $ATM $ID $SIMTYPE 
            done
            continue
       fi
       ######################
       # combine effective areas
       if [[ $IRFTYPE == "COMBINEEFFECTIVEAREAS" ]]; then
            for ID in $RECID; do
                for CUTS in ${CUTLIST[@]}; do
                    echo "combine effective areas $CUTS"
                   ./IRF.combine_effective_area_parts.sh $CUTS $VX $ATM $ID $SIMTYPE $AUX
                done # cuts
            done
            continue
        fi
        for ZA in ${ZENITH_ANGLES[@]}; do
            ######################
            # train MVA for angular resolution
            if [[ $IRFTYPE == "TRAINMVANGRES" ]]; then
               if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
                   ./IRF.trainTMVAforAngularReconstruction.sh $VX $ATM $ZA 200 $SIMTYPE
               elif [[ ${SIMTYPE:0:4} = "CARE" ]]; then
                   ./IRF.trainTMVAforAngularReconstruction.sh $VX $ATM $ZA 170 $SIMTYPE
               fi
               continue
            fi
            for NOISE in ${NSB_LEVELS[@]}; do
                for WOBBLE in ${WOBBLE_OFFSETS[@]}; do
                    echo "Now processing epoch $VX, atmo $ATM, zenith angle $ZA, wobble $WOBBLE, noise level $NOISE"
                    ######################
                    # run simulations through evndisp
                    if [[ $IRFTYPE == "EVNDISP" ]] || [[ $IRFTYPE == "MVAEVNDISP" ]]; then
                       if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
                          SIMDIR=$VERITAS_DATA_DIR/simulations/"$VX"_FLWO/grisu/ATM"$ATM"
                       elif [[ ${SIMTYPE:0:13} = "CARE_June1425" ]]; then
                          SIMDIR=$VERITAS_DATA_DIR/simulations/"$VX"_FLWO/CARE_June1425/
                       elif [[ ${SIMTYPE:0:4} = "CARE" ]]; then
                          SIMDIR=$VERITAS_DATA_DIR/simulations/"$VX"_FLWO/${SIMTYPE}
                       fi
                        ./IRF.evndisp_MC.sh $SIMDIR $VX $ATM $ZA $WOBBLE $NOISE $SIMTYPE
                    ######################
                    # make tables
                    elif [[ $IRFTYPE == "MAKETABLES" ]]; then
                        for ID in $RECID; do
                           ./IRF.generate_lookup_table_parts.sh $VX $ATM $ZA $WOBBLE $NOISE $ID $SIMTYPE
                        done #recid
                    ######################
                    # analyse table files
                    elif [[ $IRFTYPE == "ANALYSETABLES" ]]; then
                        TFIL="${TABLEFILE/VX/$VX}"
                        # note: the IDs dependent on what is written in EVNDISP.reconstruction.runparameter
                        for ID in $RECID; do
                            if [[ $ID == "0" ]] || [[ $ID == "2" ]] || [[ $ID == "3" ]] || [[ $ID == "4" ]] || [[ $ID == "5" ]] || [[ $ID == "6" ]]; then
			                    METH="GEO"
				                TFILID=$TFIL$METH
			                elif [[ $ID == "1" ]] || [[ $ID == "7" ]] || [[ $ID == "8" ]] || [[ $ID == "9" ]] || [[ $ID == "10" ]]; then 
				                METH="DISP"
				                TFILID=$TFIL$METH
			                fi
			                ./IRF.mscw_energy_MC.sh $TFILID $VX $ATM $ZA $WOBBLE $NOISE $ID $SIMTYPE
                        done
                    ######################
                    # analyse effective areas
                    elif [[ $IRFTYPE == "EFFECTIVEAREAS" ]]; then
                        for ID in $RECID; do
                            for CUTS in ${CUTLIST[@]}; do
                                echo "effective areas $CUTS"
                               ./IRF.generate_effective_area_parts.sh $CUTS $VX $ATM $ZA $WOBBLE $NOISE $ID $SIMTYPE
                            done #cuts
                        done #recID
                    fi
                done #wobble
            done #noise
        done #ZA
    done #ATM
done  #VX

exit

