#!/bin/bash
# script to analyse data files with anasum (parallel analysis) from a simple run list

if [[ $# < 4 ]]; then
# begin help message
echo "
ANASUM parallel data analysis: submit jobs using a simple run list

ANALYSIS.anasum_parallel_from_runlist.sh <run list> <output directory> <cut set> <background model> [run parameter file] [mscw directory]

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

# cut definitions (note: VX to be replaced later in script)
if [[ $CUTS == *super* ]]; then
    CUTFILE="ANASUM.GammaHadron-Cut-NTel2-PointSource-SuperSoftSpectrum.dat"
#    EFFAREA="effArea-d20131031-cut-N2-Point-005CU-SuperSoft-ATM$ATMO-VX-T1234-d20131115.root"
    EFFAREA="effArea-GRISU-140531-Cut-NTel2-PointSource-SuperSoftSpectrum-VX-ATM$ATMO-T1234.root"
    RADACC="radialAcceptance-d20131115-cut-N2-Point-005CU-SuperSoft-VX-T1234.root"
elif [[ $CUTS == *open* ]]; then
    CUTFILE="ANASUM.GammaHadron-Cut-NTel2-PointSource-Open.dat"
############### ---- not updated ---- ##############
#    EFFAREA="effArea-d20131031-cut-N2-Point-005CU-Open-ATM$ATMO-VX-T1234-d20131115.root"
    EFFAREA="effArea-140512-Cut-NTel2-PointSource-Open-VX-ATM$ATMO-T1234.root"
    RADACC="radialAcceptance-d20131115-cut-N2-Point-005CU-Open-VX-T1234.root"
elif [[ $CUTS == *soft* ]]; then
    CUTFILE="ANASUM.GammaHadron-Cut-NTel3-PointSource-SoftSpectrum.dat"
#    EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Soft-ATM$ATMO-VX-T1234-d20131115.root"
    EFFAREA="effArea-GRISU-140531-Cut-NTel3-PointSource-SoftSpectrum-VX-ATM$ATMO-T1234.root"
    RADACC="radialAcceptance-d20131115-cut-N3-Point-005CU-Soft-VX-T1234.root"
elif [[ $CUTS = *moderate* ]]; then
    CUTFILE="ANASUM.GammaHadron-Cut-NTel3-PointSource-ModerateSpectrum.dat"
    EFFAREA="effArea-GRISU-140531-Cut-NTel3-PointSource-ModerateSpectrum-VX-ATM$ATMO-T1234.root"
    RADACC="radialAcceptance-d20131115-cut-N3-Point-005CU-Moderate-V5-T1234.root"
elif [[ $CUTS = *hard* ]]; then
    CUTFILE="ANASUM.GammaHadron-Cut-NTel3-PointSource-HardSpectrum.dat"
#    EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Hard-ATM$ATMO-VX-T1234-d20131115.root"
#    RADACC="radialAcceptance-d20131115-cut-N3-Point-005CU-Hard-VX-T1234.root"
#    EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Moderate-ATM$ATMO-VX-T1234-d20131115.root"
    EFFAREA="effArea-140512-Cut-NTel3-PointSource-HardSpectrum-VX-ATM$ATMO-T1234.root"
    EFFAREA="effArea-GRISU-140531-Cut-NTel3-PointSource-HardSpectrum-VX-ATM$ATMO-T1234.root"
    RADACC="radialAcceptance-d20131115-cut-N3-Point-005CU-Moderate-V5-T1234.root"
else
    echo "ERROR: unknown cut definition: $CUTS"
    exit 1
fi

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
    echo $RUN
    EPOCH=`getRunArrayVersion $RUN`
    ATMO=`getRunAtmosphere $RUN`
    
    # do string replacements
    EFFAREA=${EFFAREA/VX/$EPOCH}
    EFFAREA=${EFFAREA/ATMXX/$ATMO}
    RADACC=${RADACC/VX/$EPOCH}
    
    # write line to file
    echo "* $RUN $RUN 0 $CUTFILE $BM $EFFAREA $BMPARAMS $RADACC" >> $ANARUNLIST
done

# submit the job
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/ANALYSIS.anasum_parallel"
$SUBSCRIPT.sh $ANARUNLIST $INDIR $ODIR $RUNP

exit
