#!/bin/bash
# IRF production script (VERITAS)

if [ $# -lt 3 ]; then
# begin help message
echo "
IRF generation: produce a full set of instrument response functions (IRFs)

IRF.production.sh <sim directory> <sim type> <IRF type> [epoch] [atmosphere]
 [Rec ID] [cuts list file]

required parameters:

    <sim directory>         directory containing simulation VBF files
    
    <sim type>              original VBF file simulation type (e.g. GRISU, CARE)
    
    <IRF type>              type of instrument response function to produce
                            (e.g. TABLE, EFFECTIVEAREA, RADIALACCEPTANCE)
    
optional parameters:
    
    [epoch]                 array epoch(s) (e.g., V4, V5, V6)
                            (default: \"V4 V5 V6\")
                            
    [atmosphere]            atmosphere model(s) (21 = winter, 22 = summer)
                            (default: \"21 22\")
                            
    [Rec ID]                reconstruction ID(s) (default: \"0 1 2 3 4\")
                            (see EVNDISP.reconstruction.runparameter)
    
    [cuts list file]        file containing one gamma/hadron cuts file per line
                            (default: hard-coded standard EventDisplay cuts)

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
SIMDIR=$1
SIMTYPE=$2
IRFTYPE=$3
[[ "$4" ]] && EPOCH=$4 || EPOCH="V4 V5 V6"
[[ "$5" ]] && ATMOS=$5 || ATMOS="21 22"
[[ "$6" ]] && RECID=$6 || RECID="0 1 2 3 4"
[[ "$7" ]] && CUTSLISTFILE=$7 || CUTSLISTFILE=""

# Run sims through evndisp and mscw_energy
bash $(dirname "$0")"/IRF.evndisp_MC.sh $SIMDIR $EPOCH $ATMOS $SIMTYPE"
sleep 60    # wait for scripts to be submitted
while JOBS_REMAINING=`qstat -u $USER | grep evndisp_MC | wc -l`
      echo $JOBS_REMAINING
      [[ $JOBS_REMAINING > 0 ]]; do
        
    sleep 60


# Set cuts
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

for CUTS in ${CUTLIST[@]}; do
    for VX in $EPOCH; do
        for ATM in $ATMOS; do
            for ID in $RECID; do
                echo "Now generating radial acceptance for $CUTS, epoch $VX, ATM$ATM, Rec ID $ID"

                # make tables
                if [[ $IRFTYPE == "MAKETABLES" ]]
                then
                echo "  ./VTS.MSCW_ENERGY.sub_make_tables.sh $T 0 $W $A $SIMTYPE"
                fi

                # analyse table files
                if [[ $IRFTYPE == "ANALYSETABLES" ]]; then
                    ./VTS.MSCW_ENERGY.sub_analyse_MC_VBF.sh $T $I $W $A $SIMTYPE
                fi

                # analyse effective areas
                if [[ $IRFTYPE == "EFFECTIVEAREAS" ]]; then

                fi
            done
        done
    done
done

exit
