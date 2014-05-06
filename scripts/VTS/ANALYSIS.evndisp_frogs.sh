#!/bin/bash
# script to run eventdisplay analysis with FROGS

# qsub parameters
h_cpu=14:29:00; h_vmem=2000M; tmpdir_size=10G

if [ $# -lt 2 ]; then
# begin help message
echo "
EVNDISP data analysis: evndisp FROGS analysis for a simple run list

ANALYSIS.evndisp_frogs.sh <runlist> <mscw directory> <output directory>
 [calibration] [VPM]

required parameters:

    <runlist>               simple run list with one run number per line

    <mscw directory>        directory which contains mscw_energy files
    
    <output directory>      directory where output ROOT files will be stored

optional parameters:
    
    [calibration]
          1                 pedestal & average tzero calculation (default)
          2                 pedestal calculation only
          3                 average tzero calculation only

    [VPM]                   set to 0 to switch off (default is on)

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
MSCWDIR=$2
ODIR=$3
mkdir -p $ODIR
[[ "$4" ]] && CALIB=$4 || CALIB=1
[[ "$5" ]] && VPM=$5 || VPM=1

# Read runlist
if [ ! -f "$RLIST" ] ; then
    echo "Error, runlist $RLIST not found, exiting..."
    exit 1
fi
FILES=`cat $RLIST`

# Output directory for error/output
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EVNDISP.ANADATA"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.evndisp_frogs_sub"

# loop over all files in files loop
for AFILE in $FILES
do
    echo "Now starting run $AFILE"
    FSCRIPT="$LOGDIR/EVN.data-$AFILE"

    sed -e "s|RUNFILE|$AFILE|" \
        -e "s|CALIBRATIONOPTION|$CALIB|" \
        -e "s|OUTPUTDIRECTORY|$ODIR|"  \
        -e "s|MSCWDIRECTORY|$MSCWDIR|" \
        -e "s|USEVPMPOINTING|$VPM|" $SUBSCRIPT.sh > $FSCRIPT.sh

    chmod u+x $FSCRIPT.sh
    echo $FSCRIPT.sh

    # run locally or on cluster
    SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
    SUBC=`eval "echo \"$SUBC\""`
    if [[ $SUBC == *qsub* ]]; then
        $SUBC $FSCRIPT.sh
    elif [[ $SUBC == *parallel* ]]; then
        echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
    fi
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit

