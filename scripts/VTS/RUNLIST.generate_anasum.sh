#!/bin/bash
# script to generate an anasum run list from a simple run list
# Author: Scott Griffiths

if [ $# -lt 4 ]; then
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

# Parse command line arguments
RLIST=$1
ODIR=$2
mkdir -p $ODIR
CUTS=$3
BACKGND=$4

# cut definitions (note: $ATM and $VX to be replaced later in the script"
if [[ $CUTS == *super* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-SuperSoft.dat'
    EFFAREA='effArea-d20131031-cut-N2-Point-005CU-SuperSoft-ATM$ATM-$VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N2-Point-005CU-SuperSoft-$VX-T1234.root'
elif [[ $CUTS == *open* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20131031-cut-N2-Point-005CU-Open.dat'
    EFFAREA='effArea-d20131031-cut-N2-Point-005CU-Open-ATM$ATM-$VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N2-Point-005CU-Open-$VX-T1234.root'
elif [[ $CUTS == *soft* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft.dat'
    EFFAREA='effArea-d20131031-cut-N3-Point-005CU-Soft-ATM$ATM-$VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N3-Point-005CU-Soft-$VX-T1234.root'
elif [[ $CUTS = *moderate* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate.dat'
    EFFAREA='effArea-d20131031-cut-N3-Point-005CU-Moderate-ATM$ATM-$VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N3-Point-005CU-Moderate-$VX-T1234.root'
elif [[ $CUTS = *hard* ]]; then
    CUTFILE='ANASUM.GammaHadron.d20120909-cut-N3-Point-005CU-Hard.dat'
    EFFAREA='effArea-d20131031-cut-N3-Point-005CU-Hard-ATM$ATM-$VX-T1234-d20131115.root'
    RADACC='radialAcceptance-d20131115-cut-N3-Point-005CU-Hard-$VX-T1234.root'
else
    echo "ERROR: unknown cut definition: $CUTS"
    exit 1
fi

# Prepend location within VERITAS_EVNDISP_AUX_DIR
CUTFILE="$VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles/$CUTFILE"
EFFAREA="$VERITAS_EVNDISP_AUX_DIR/EffectiveAreas/$EFFAREA"
RADACC="$VERITAS_EVNDISP_AUX_DIR/RadialAcceptances/$RADACC"

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

# Get VERITAS database URL from EVNDISP.global.runparameter file
MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`
if [ ! -n "$MYSQLDB" ]; then
    echo "* DBSERVER param not found in \$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter!"
    exit 1
fi

# Read runlist file
if [ ! -f "$RLIST" ]; then
    echo "Error, simple runlist $RLIST not found, exiting..."
    exit 1
fi
OLDIFS=$IFS
IFS=$'\r\n' RUNS=($(cat $RLIST))
IFS=$OLDIFS

# Convert RUNS to a comma-separated tuple
RUN_IDS=`echo ${RUNS[@]} | sed "s| |, |g"`

# Get run dates from the database
RUN_DATES=($(mysql -u readonly -h $MYSQLDB -A -e "select left(data_start_time, 10) from VERITAS.tblRun_Info where run_id in ($RUN_IDS)"))
RUN_DATES=("${RUN_DATES[@]:2}")

#########################################
# make anasum run list
ANARUNLIST="$ODIR/$CUTS.anasum.dat"
rm -f $ANARUNLIST

# run list header
echo "* VERSION 6" >> $ANARUNLIST
echo "" >> $ANARUNLIST

for (( i=0; i < ${#RUNS[@]}; i++ )); do
    # set run correct epoch (V4, V5, V6)
    if [[ ${RUNS[i]} < 46642 ]]; then
        VX="V4"
    elif [[ ${RUNS[i]} > 63373 ]]; then
        VX="V6"
    else
        VX="V5"
    fi
    
    # set correct atmosphere
    RUNDATE=$(echo ${RUN_DATES[i]} | cut -d' ' -f 2 | sed "s|-||g")
    RUNMONTH=${RUNDATE:4:2}
    if (( $RUNDATE > 20071026 && $RUNDATE < 20080420 ||
          $RUNDATE > 20081113 && $RUNDATE < 20090509 ||
          $RUNDATE > 20091102 && $RUNDATE < 20100428 ||
          $RUNDATE > 20101023 && $RUNDATE < 20110418 ||
          $RUNDATE > 20111110 && $RUNDATE < 20120506 ||
          $RUNDATE > 20121029 && $RUNDATE < 20130425 )); then
        # use winter atmosphere
        ATM=21
    elif (( $RUNDATE >= 20130425 || $RUNDATE <= 20071026 )); then
        # don't have specific dates for summer/winter boundary, so we will
        # generalize that the months May through October inclusive is 'summer'
        if (( $RUNMONTH >= 5 && $RUNMONTH <= 10 )); then
            ATM=22
        # November through April inclusive is 'winter'
        elif (( $RUNMONTH <= 4 || $RUNMONTH >= 11 )); then
            ATM=21
        fi
    else
        echo "ERROR! Atmosphere could not be assigned for run date: $RUNDATE"
        exit 1
    fi
    
    # Replace EFFAREA and RADACC strings with correct array and atmosphere
    EFFAREA=`eval "echo \"$EFFAREA\""`
    RADACC=`eval "echo \"$RADACC\""`
    
    # write line to anasum runlist
    echo "* ${RUNS[i]} ${RUNS[i]} 0 $CUTFILE $BM $EFFAREA $BMPARAMS $RADACC" >> $ANARUNLIST
done

echo "Anasum run list written to: $ANARUNLIST"

exit
