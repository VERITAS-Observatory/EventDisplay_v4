#!/bin/bash
# IRF production script (VERITAS)
#
# full list of epcochs:
# V6_2012_2013 V6_2013_2014 V6_2014_2015 V6_2015_2016 V6_2016_2017 V6_2017_2018 V6_2018_2019 V6
#
#

if [ $# -lt 2 ]; then
# begin help message
echo "
IRF generation: produce a full set of instrument response functions (IRFs)

IRF.production.sh <sim type> <IRF type> [epoch] [atmosphere] [Rec ID] [BDT cuts] [cuts list file] [sim directory]

required parameters:

    <sim type>              original VBF file simulation type (e.g. GRISU-SW6, CARE_June1425)
    
    <IRF type>              type of instrument response function to produce
                            (e.g. EVNDISP, MAKETABLES, COMBINETABLES,
                             ANALYSETABLES, EFFECTIVEAREAS, COMBINEEFFECTIVEAREAS,
                             MVAEVNDISP, TRAINMVANGRES )
    
optional parameters:
    
    [epoch]                 array epoch(s) (e.g., V4, V5, V6)
                            (default: \"V4 V5 V6\")
                            
    [atmosphere]            atmosphere model(s) (21 = winter, 22 = summer)
                            (default: \"21 22\")
                            
    [Rec ID]                reconstruction ID(s) (default: \"0 2 3 4 5\")
                            (see EVNDISP.reconstruction.runparameter)

    [BDT cuts]              using cuts list for BDT cuts (e.g. 0=not used, 1=used)
                            (default: \"2\")
    
    [cuts list file]        file containing one gamma/hadron cuts file per line
                            (default: hard-coded standard EventDisplay cuts)

    [sim directory]         directory containing simulation VBF files

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# We need to be in the IRF.production.sh directory so that subscripts are called
# (we call them ./).
olddir=$(pwd)
cd $(dirname "$0")

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
SIMTYPE=$1
IRFTYPE=$2
[[ "$3" ]] && EPOCH=$3 || EPOCH="V4 V5 V6"
[[ "$4" ]] && ATMOS=$4 || ATMOS="21 22"
[[ "$5" ]] && RECID=$5 || RECID="0 2 3 4 5"
[[ "$6" ]] && BDTCUTS=$6 || BDTCUTS="1"
[[ "$7" ]] && CUTSLISTFILE=$7 || CUTSLISTFILE=""
[[ "$8" ]] && SIMDIR=$8 || SIMDIR=""
# evndisplay version
IRFVERSION=`$EVNDISPSYS/bin/printRunParameter --version | tr -d .| sed -e 's/[a-Z]*$//'`

# version string for aux files
AUX="auxv01"

# number of events per evndisp analysis
NEVENTS="-1"

# Default run parameter file for evndisp analysis
ACUTS="EVNDISP.reconstruction.runparameter"

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
elif [ "${SIMTYPE}" = "CARE_June1702" ]; then
    # CARE_June1702 simulation parameters
    if [[ $ATMOS == "62" ]]; then
        ZENITH_ANGLES=( 00 20 30 35 40 45 50 )
    else
        ZENITH_ANGLES=( 00 20 30 35 40 45 50 55 )
    fi
    NSB_LEVELS=( 50 75 100 130 160 200 250 300 350 400 450 )
    WOBBLE_OFFSETS=( 0.5 )
    NEVENTS="15000000"
elif [ "${SIMTYPE}" = "CARE_RedHV" ]; then
    ZENITH_ANGLES=$(ls $VERITAS_DATA_DIR/simulations/V6_FLWO/CARE_June1702_RHV/*.zst | awk -F "_zen" '{print $2}' | awk -F "deg." '{print $1}' | sort | uniq) 
    NSB_LEVELS=$(ls $VERITAS_DATA_DIR/simulations/V6_FLWO/CARE_June1702_RHV/*.zst | awk -F "wob_" '{print $2}' | awk -F "MHz." '{print $1}' | sort | uniq)
    WOBBLE_OFFSETS=( 0.5 ) 
elif [[ "${SIMTYPE}" = "CARE_June2020" ]]; then
    DDIR="$VERITAS_DATA_DIR/simulations/V6_FLWO/${SIMTYPE}/Atmosphere${ATMOS}/"
    ZENITH_ANGLES=$(ls $DDIR/ | awk -F "Zd" '{print $2}' | sort | uniq)
    set -- $ZENITH_ANGLES
    NSB_LEVELS=$(ls ${DDIR}/Zd$1/merged/Data/*.zst | awk -F "_" '{print $10}' |  awk -F MHz '{print $1}' | sort -u)
    WOBBLE_OFFSETS=$(ls ${DDIR}/Zd$1/merged/Data/*.zst | awk -F "_" '{print $9}' |  awk -F wob '{print $1}' | sort -u)
    NEVENTS="15000000"
elif [ ${SIMTYPE:0:4} = "CARE" ]; then
    # Older CARE simulation parameters
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

# Set gamma/hadron cuts
if [[ $CUTSLISTFILE != "" ]]; then
    if [ ! -f $CUTSLISTFILE ]; then
        echo "Error, cuts list file $CUTSLISTFILE not found, exiting..."
        exit 1
    fi
    # read file containing list of cuts
    IFS=$'\r\n' CUTLIST=($(cat $CUTSLISTFILE))
else
    if [[ $BDTCUTS == "0"  ]]; then
    # default list of cuts
        CUTLIST="ANASUM.GammaHadron-Cut-NTel2-PointSource-Moderate-TMVA-Preselection.dat
                 ANASUM.GammaHadron-Cut-NTel2-PointSource-Soft-TMVA-Preselection.dat
                 ANASUM.GammaHadron-Cut-NTel2-PointSource-Soft.dat
                 ANASUM.GammaHadron-Cut-NTel2-PointSource-Moderate.dat 
                 ANASUM.GammaHadron-Cut-NTel3-PointSource-Hard.dat"
    else
    #BDT TMVA list of cuts
        CUTLIST="ANASUM.GammaHadron-Cut-NTel2-PointSource-Moderate-TMVA-BDT.dat 
                 ANASUM.GammaHadron-Cut-NTel2-PointSource-Soft-TMVA-BDT.dat 
                 ANASUM.GammaHadron-Cut-NTel2-PointSource-Hard-TMVA-BDT.dat
                 ANASUM.GammaHadron-Cut-NTel3-PointSource-Hard-TMVA-BDT.dat"
    fi
fi
CUTLIST=`echo $CUTLIST |tr '\r' ' '`

############################################################
# loop over complete parameter space and submit production
for VX in $EPOCH; do
    for ATM in $ATMOS; do
       ######################
       # set lookup table files
       # (METHOD "DISP" or "GEO" is set later)
       TABLECOM="table-${IRFVERSION}-${AUX}-${SIMTYPE}-ATM${ATM}-${VX}-"
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
                          SIMDIR=$VERITAS_DATA_DIR/simulations/"${VX:0:2}"_FLWO/CARE_June1425/
                       elif [[ ${SIMTYPE:0:10} = "CARE_RedHV" ]]; then
                          SIMDIR=$VERITAS_DATA_DIR/simulations/"${VX:0:2}"_FLWO/CARE_June1702_RHV/
                       elif [[ ${SIMTYPE:0:13} = "CARE_June2020" ]]; then
                          SIMDIR="$VERITAS_DATA_DIR/simulations/V6_FLWO/${SIMTYPE}/Atmosphere${ATMOS}/"
                       elif [[ ${SIMTYPE:0:4} = "CARE" ]]; then
                          SIMDIR=$VERITAS_DATA_DIR/simulations/"${VX:0:2}"_FLWO/${SIMTYPE}
                       fi
                       ./IRF.evndisp_MC.sh $SIMDIR $VX $ATM $ZA $WOBBLE $NOISE $SIMTYPE $ACUTS 1 $NEVENTS
                    ######################
                    # make tables
                    elif [[ $IRFTYPE == "MAKETABLES" ]]; then
                        for ID in $RECID; do
                           ./IRF.generate_lookup_table_parts.sh $VX $ATM $ZA $WOBBLE $NOISE $ID $SIMTYPE
                        done #recid
                    ######################
                    # analyse table files
                    elif [[ $IRFTYPE == "ANALYSETABLES" ]]; then
                        TFIL="${TABLECOM}"
                        # note: the IDs dependent on what is written in EVNDISP.reconstruction.runparameter
                        # warning: do not mix disp and geo
                        METH="NOTSET"
                        for ID in $RECID; do
                            if [[ $ID == "0" ]] || [[ $ID == "2" ]] || [[ $ID == "3" ]] || [[ $ID == "4" ]] || [[ $ID == "5" ]] || [[ $ID == "6" ]]; then
                               if [[ $METH != "NOTSET" ]] && [[ $METH != "GEO" ]]; then
                                   echo "invalid RECID combination, do not mix GEO and DISP"
                                   exit
                               fi
                               METH="GEO"
			    elif [[ $ID == "1" ]] || [[ $ID == "7" ]] || [[ $ID == "8" ]] || [[ $ID == "9" ]] || [[ $ID == "10" ]]; then 
                               if [[ $METH != "NOTSET" ]] || [[ $METH != "DISP" ]]; then
                                   echo "invalid RECID combination, do not mix GEO and DISP"
                                   exit
                               fi
			       METH="DISP"
			    fi
                        done
                        TFILID=$TFIL$METH
			./IRF.mscw_energy_MC.sh $TFILID $VX $ATM $ZA $WOBBLE $NOISE "$RECID" $SIMTYPE
                    ######################
                    # analyse effective areas
                    elif [[ $IRFTYPE == "EFFECTIVEAREAS" ]]; then
                        for ID in $RECID; do
                            #./IRF.generate_effective_area_parts.sh "$CUTLIST" $VX $ATM $ZA $WOBBLE $NOISE $ID $SIMTYPE
                            for CUTS in ${CUTLIST[@]}; do
                                echo "combine effective areas $CUTS"
                               ./IRF.generate_effective_area_parts.sh $CUTS $VX $ATM $ZA $WOBBLE $NOISE $ID $SIMTYPE
                            done # cuts
                        done #recID
                    fi
                done #wobble
            done #noise
        done #ZA
    done #ATM
done  #VX

# Go back to the original user directory.
cd $olddir
exit

