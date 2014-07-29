#!/bin/bash
# script to analyse data files with anasum (parallel analysis) from a simple run list

if [[ $# < 4 ]]; then
# begin help message
echo "
ANASUM parallel data analysis: submit jobs using a simple run list

ANALYSIS.anasum_parallel_from_runlist.sh <run list> <output directory> <cut set> <background model> [run parameter file] [mscw directory] [sim type] [baseID]

required parameters:

    <run list>              simple runlist with a single run number per line
        
    <output directory>      anasum output files are written to this directory
                        
    <cut set>               hardcoded cut sets predefined in the script
                            (e.g., soft, moderate, etc.)
    
    <background model>      background model
                            (RE = reflected region, RB = ring background)
    
optional parameters:

    [run parameter file]    anasum run parameter file (located in 
                            \$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/;
                            default is ANASUM.runparameter)

    [mscw directory]        directory containing the mscw.root files

    [sim type]              use IRFs derived from this simulation type (e.g. GRISU, CARE)

    [baseID]                (should be 0 or 11 in the current release candidate)

IMPORTANT! Run ANALYSIS.anasum_combine.sh once all parallel jobs have finished!

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Load runlist functions
source "$EVNDISPSYS/scripts/VTS/helper_scripts/RUNLIST.run_info_functions.sh"

# Parse command line arguments
RLIST=$1
ODIR=$2
CUTS=$3
BACKGND=$4
[[ "$5" ]] && RUNP=$5  || RUNP="ANASUM.runparameter"
[[ "$6" ]] && INDIR=$6 || INDIR="$VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION/RecID0"
[[ "$7" ]] && SIMTYPE=$7 || SIMTYPE="GRISU"
[[ "$8" ]] && BASEID=$8 || BASEID="0"

###########################
# IRFs
IRFVERSION="v447-auxv01"

# cut definitions (note: VX to be replaced later in script)
if [[ $CUTS == *superhard* ]]; then
    CUT="NTel3-PointSource-SuperHard"
elif [[ $CUTS == *super* ]]; then
    CUT="NTel2-PointSource-SuperSoftSpectrum"
elif [[ $CUTS == *moderateopen* ]]; then
    CUT="NTel2-PointSource-ModerateOpen"
elif [[ $CUTS == *softopen* ]]; then
    CUT="NTel2-PointSource-SoftOpen"
elif [[ $CUTS == *hardopen* ]]; then
    CUT="NTel2-PointSource-HardOpen"
elif [[ $CUTS == *softExt* ]]; then
    CUT="NTel2-ExtendedSource-Soft"
elif [[ $CUTS == *soft* ]]; then
    CUT="NTel2-PointSource-Soft"
elif [[ $CUTS = *moderate2tel* ]]; then
    CUT="NTel2-PointSource-Moderate"
elif [[ $CUTS = *moderateExt2tel* ]]; then
    CUT="NTel2-ExtendedSource-Moderate"
elif [[ $CUTS = *moderate3tel* ]]; then
    CUT="NTel3-PointSource-Moderate"
elif [[ $CUTS = *hard2tel* ]]; then
    CUT="NTel2-PointSource-Hard"
elif [[ $CUTS = *hard3tel* ]]; then
    CUT="NTel3-PointSource-Hard"
elif [[ $CUTS = *hard2Exttel* ]]; then
    CUT="NTel2-ExtendedSource-Moderate"
else
    echo "ERROR: unknown cut definition: $CUTS"
    exit 1
fi
CUTFILE="ANASUM.GammaHadron-Cut-${CUT}.dat"
EFFAREA="effArea-${IRFVERSION}-${SIMTYPE}-Cut-${CUT}-ID${BASEID}-VX-ATMXX-TX.root"
RADACC="radialAcceptance-${IRFVERSION}-Cut-${CUT}-ID${BASEID}-VX-TX.root"
# preliminary: use ID0 for all data
RADACC="radialAcceptance-${IRFVERSION}-Cut-${CUT}-ID0-VX-TX.root"

echo $CUTFILE
echo $EFFAREA
echo $RADACC

# background model parameters
if [[ "$BACKGND" == *RB* ]]; then
    BM="RB"
    BMPARAMS="0.6 20"
elif [[ "$BACKGND" == *RE* ]]; then
    BM="RE"
    BMPARAMS="0.1 2 6"
else
    echo "ERROR: unknown background model: $BACKGND"
    echo "Allowed values are: RE, RB"
    exit 1
fi

# Check that run list exists
if [[ ! -f "$RLIST" ]]; then
    echo "Error, simple runlist $RLIST not found, exiting..."
    exit 1
fi

# Check that run parameter file exists
if [[ "$RUNP" == `basename $RUNP` ]]; then
    RUNP="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/$RUNP"
fi
if [ ! -f "$RUNP" ]; then
    echo "Error, anasum run parameter file not found, exiting..."
    exit 1
fi

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/ANASUM.ANADATA"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# output directory for anasum products
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR

#########################################
# make anasum run list
ANARUNLIST="$ODIR/$CUTS.anasum.dat"
rm -f $ANARUNLIST
echo "anasum run list: $ANARUNLIST"

# run list header
echo "* VERSION 6" >> $ANARUNLIST
echo "" >> $ANARUNLIST

RUNS=`cat $RLIST`

for RUN in ${RUNS[@]}; do
    # get array epoch and atmosphere for the run
    EPOCH=`getRunArrayVersion $RUN`
    ATMO=`getRunAtmosphere $RUN $INDIR/$RUN.mscw.root`
    # ATMO="21"
    if [[ $ATMO == "error" ]]; then
       echo "error finding atmosphere; skipping run $RUN"
       continue
    fi
    echo "RUN $RUN at epoch $EPOCH and atmosphere $ATMO"
    TELTOANA=`$EVNDISPSYS/bin/printRunParameter $INDIR/$RUN.mscw.root -teltoana`
    TELTOANA="T$TELTOANA"
    
    # do string replacements
    EFFAREA=${EFFAREA/VX/$EPOCH}
    EFFAREA=${EFFAREA/TX/$TELTOANA}
    EFFAREA=${EFFAREA/XX/$ATMO}
    RADACC=${RADACC/VX/$EPOCH}
    RADACC=${RADACC/TX/$TELTOANA}
    
    # write line to file
    echo "* $RUN $RUN 0 $CUTFILE $BM $EFFAREA $BMPARAMS $RADACC" >> $ANARUNLIST
    echo "* $RUN $RUN 0 $CUTFILE $BM $EFFAREA $BMPARAMS $RADACC"
done

# submit the job
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/ANALYSIS.anasum_parallel"
$SUBSCRIPT.sh $ANARUNLIST $INDIR $ODIR $RUNP

exit
