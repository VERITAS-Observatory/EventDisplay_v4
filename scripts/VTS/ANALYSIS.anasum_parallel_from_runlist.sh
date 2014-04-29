#!/bin/bash
# script to analyse data files with anasum (parallel analysis) from a simple run list
# Author: Gernot Maier

if [ $# -lt 6 ]; then
# begin help message
echo "
ANASUM parallel data analysis: submit jobs using a simple run list

ANALYSIS.anasum_parallel_from_runlist.sh <run list> <mscw directory>
 <output directory> <run parameter file> <cut set> <background model>
 [atmosphere]

required parameters:

    <run list>              simple runlist with a single run number per line
        
    <mscw directory>        directory containing the mscw.root files
        
    <output directory>      anasum output files are written to this directory
        
    <run parameter file>    anasum run parameter file
                            (in \$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/;
                            see ANASUM.runparameter.dat for an example)
                        
    <cut set>               hardcoded cut sets predefined in the script
                            (e.g., soft, moderate, etc.)
    
    <background model>      background model
                            (RE = reflected region, RB = ring background)
    
optional parameters:
    
    [atmosphere]            atmosphere (default = 21, i.e., winter)

IMPORTANT! Run ANALYSIS.anasum_combine.sh once all parallel jobs have finished!

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
RLIST=$1
INDIR=$2
ODIR=$3
RUNP=$4
CUTS=$5
BACKGND=$6
[[ "$7" ]] && ATMO=$7 || ATMO=21

# cut definitions (note: VX to be replaced later in script)
if [[ $CUTS == *super* ]]; then
    CUTFILE="ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-SuperSoft.dat"
    EFFAREA="effArea-d20131031-cut-N2-Point-005CU-SuperSoft-ATM$ATMO-VX-T1234-d20131115.root"
    RADACC="radialAcceptance-d20131115-cut-N2-Point-005CU-SuperSoft-VX-T1234.root"
elif [[ $CUTS == *open* ]]; then
    CUTFILE="ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open.dat"
    EFFAREA="effArea-d20131031-cut-N2-Point-005CU-Open-ATM$ATMO-VX-T1234-d20131115.root"
    RADACC="radialAcceptance-d20131115-cut-N2-Point-005CU-Open-VX-T1234.root"
elif [[ $CUTS == *soft* ]]; then
    CUTFILE="ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft.dat"
    EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Soft-ATM$ATMO-VX-T1234-d20131115.root"
    RADACC="radialAcceptance-d20131115-cut-N3-Point-005CU-Soft-VX-T1234.root"
elif [[ $CUTS = *moderate* ]]; then
    CUTFILE="ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate.dat"
    EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Moderate-ATM$ATMO-VX-T1234-d20131115.root"
    RADACC="radialAcceptance-d20131115-cut-N3-Point-005CU-Moderate-V5-T1234.root"
elif [[ $CUTS = *hard* ]]; then
    CUTFILE="ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Hard.dat"
    EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Hard-ATM$ATMO-VX-T1234-d20131115.root"
    RADACC="radialAcceptance-d20131115-cut-N3-Point-005CU-Hard-VX-T1234.root"
    EFFAREA="effArea-d20131031-cut-N3-Point-005CU-Moderate-ATM$ATMO-VX-T1234-d20131115.root"
    RADACC="radialAcceptance-d20131115-cut-N3-Point-005CU-Moderate-V5-T1234.root"
else
    echo "ERROR: unknown cut definition: $CUTS"
    exit 1
fi

# Prepend location within VERITAS_EVNDISP_AUX_DIR
# (GM) not needed
# CUTFILE="$VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles/$CUTFILE"
# EFFAREA="$VERITAS_EVNDISP_AUX_DIR/EffectiveAreas/$EFFAREA"
# RADACC="$VERITAS_EVNDISP_AUX_DIR/RadialAcceptances/$RADACC"

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
if [ ! -f "$RLIST" ]; then
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
mkdir -p $LOGDIR

# output directory for anasum products
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
    # set run correct epoch (V4, V5, V6)
    if [[ $RUN -lt 46642 ]]; then
        EPOCH="V4"
    elif [[ $RUN -gt 63373 ]]; then
        EPOCH="V6"
    else
        EPOCH="V5"
    fi
    
    echo "* $RUN $RUN 0 $CUTFILE $BM ${EFFAREA/VX/$EPOCH} $BMPARAMS ${RADACC/VX/$EPOCH}" >> $ANARUNLIST
done

# submit the job
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/ANALYSIS.anasum_parallel"
$SUBSCRIPT.sh $ANARUNLIST $INDIR $ODIR $RUNP

exit
