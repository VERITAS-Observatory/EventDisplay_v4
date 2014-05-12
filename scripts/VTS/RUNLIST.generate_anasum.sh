#!/bin/bash
# script to generate an anasum run list from a simple run list
# Author: Scott Griffiths

if [[ $# < 4 ]]; then
# begin help message
echo "
EVNDISP runlist script: generate a VERSION 6 anasum runlist from a simple
runlist with one run per line

RUNLIST.generate_anasum.sh <run list> <output dir> <cut set> <background model>

required parameters:

    <run list>              simple runlist with a single run number per line
    
    <output directory>      location where output anasum list will be written;
                            file name = <output directory>/<cut set>.anasum.dat

    <cut set>               hardcoded cut sets predefined in the script
                            (e.g., soft, moderate, etc.)
    
    <background model>      background model
                            (RE = reflected region, RB = ring background)
    
--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Load runlist functions
source "$EVNDISPSYS/scripts/VTS/helper_scripts/RUNLIST.run_info_functions.sh"

# Parse command line arguments
RLIST=$1
ODIR=$2
mkdir -p $ODIR
CUTS=$3
BACKGND=$4

# cut definitions (note: ATMXX and VX to be replaced later in the script"
if [[ $CUTS == *super* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-SuperSoft.dat'
    EFFAREA='effArea-d20131031-cut-N2-Point-005CU-SuperSoft-ATMXX-VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N2-Point-005CU-SuperSoft-VX-T1234.root'
elif [[ $CUTS == *open* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open.dat'
    EFFAREA='effArea-d20131031-cut-N2-Point-005CU-Open-ATMXX-VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N2-Point-005CU-Open-VX-T1234.root'
elif [[ $CUTS == *soft* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft.dat'
    EFFAREA='effArea-d20131031-cut-N3-Point-005CU-Soft-ATMXX-VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N3-Point-005CU-Soft-VX-T1234.root'
elif [[ $CUTS = *moderate* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate.dat'
    EFFAREA='effArea-d20131031-cut-N3-Point-005CU-Moderate-ATMXX-VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N3-Point-005CU-Moderate-VX-T1234.root'
elif [[ $CUTS = *hard* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20120909-cut-N3-Point-005CU-Hard.dat'
    EFFAREA='effArea-d20131031-cut-N3-Point-005CU-Hard-ATMXX-VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N3-Point-005CU-Hard-VX-T1234.root'
else
    echo "ERROR: unknown cut definition: $CUTS"
    exit 1
fi

# background model parameters
if [[ "$BACKGND" == *RB* ]]; then
    BM="1"
    BMPARAMS="0.6 20"
elif [[ "$BACKGND" == *RE* ]]; then
    BM="2"
    BMPARAMS="0.5 2 10"
else
    echo "ERROR: unknown background model: $BACKGND"
    echo "Allowed values are: RE, RB"
    exit 1
fi

# Read runlist file
if [ ! -f "$RLIST" ]; then
    echo "Error, simple runlist $RLIST not found, exiting..."
    exit 1
fi
RUNNUMS=`cat $RLIST`

#########################################
# make anasum run list
ANARUNLIST="$ODIR/$CUTS.anasum.dat"
rm -f $ANARUNLIST

# run list header
echo "* VERSION 6" >> $ANARUNLIST
echo "" >> $ANARUNLIST

for RUN in $RUNNUMS; do
    # get array epoch and atmosphere for the run
    EPOCH=`getRunArrayVersion $RUN`
    ATMO=`getRunAtmosphere $RUN`
    
    # do string replacements
    EFFAREA=${EFFAREA/VX/$EPOCH}
    EFFAREA=${EFFAREA/ATMXX/$ATMO}
    RADACC=${RADACC/VX/$EPOCH}
    
    # write line to anasum runlist
    echo "* $RUN $RUN 0 $CUTFILE $BM $EFFAREA $BMPARAMS $RADACC" >> $ANARUNLIST
done

echo "Anasum run list written to: $ANARUNLIST"

exit
