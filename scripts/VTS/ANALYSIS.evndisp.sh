#!/bin/bash
# script to run eventdisplay analysis for VTS data

# qsub parameters
h_cpu=41:29:00; h_vmem=2000M; tmpdir_size=10G

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
EVNDISP data analysis: submit jobs from a simple run list

ANALYSIS.evndisp.sh <runlist> [output directory] [calibration] [VPM] [Model3D]

required parameters:

    <runlist>               simple run list with one run number per line
    
optional parameters:
    
    [output directory]      directory where output ROOT files will be stored

    [calibration]
          1                 pedestal & average tzero calculation (default)
          2                 pedestal calculation only
          3                 average tzero calculation only

    [VPM]                   set to 0 to switch off (default is on)

    [Model3D]               set to 1 to switch on  (default is off)
    
--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# EventDisplay version
EDVERSION=`$EVNDISPSYS/bin/evndisp --version | tr -d .`

# create extra stdout for duplication of command output
# look for ">&5" below
exec 5>&1

# Parse command line arguments
RLIST=$1
[[ "$2" ]] && ODIR=$2 || ODIR="$VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION/"
mkdir -p $ODIR
[[ "$3" ]] && CALIB=$3 || CALIB=1
[[ "$4" ]] && VPM=$4   || VPM=1
[[ "$5" ]] && MODEL3D=$5 || MODEL3D=0

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
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.evndisp_sub"

#########################################
# loop over all files in files loop
for AFILE in $FILES
do
    echo "Now starting run $AFILE"
    FSCRIPT="$LOGDIR/EVN.data-$AFILE"

    sed -e "s|RUNFILE|$AFILE|"              \
        -e "s|CALIBRATIONOPTION|$CALIB|"    \
        -e "s|OUTPUTDIRECTORY|$ODIR|"       \
        -e "s|USEVPMPOINTING|$VPM|" \
        -e "s|USEMODEL3D|$MODEL3D|" $SUBSCRIPT.sh > $FSCRIPT.sh

    chmod u+x $FSCRIPT.sh
    echo $FSCRIPT.sh

    # run locally or on cluster
    SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
    SUBC=`eval "echo \"$SUBC\""`
    if [[ $SUBC == *qsub* ]]; then
        JOBID=`$SUBC $FSCRIPT.sh`
        # account for -terse changing the job number format
        if [[ $SUBC != *-terse* ]] ; then
            echo "without -terse!"      # need to match VVVVVVVV  8539483  and 3843483.1-4:2
            JOBID=$( echo "$JOBID" | grep -oP "Your job [0-9.-:]+" | awk '{ print $3 }' )
        fi
        
        echo "RUN $RUN JOBID $JOBID"
        echo "RUN $RUN SCRIPT $FSCRIPT.sh"
        if [[ $SUBC != */dev/null* ]] ; then
            echo "RUN $RUN OLOG $FSCRIPT.sh.o$JOBID"
            echo "RUN $RUN ELOG $FSCRIPT.sh.e$JOBID"
        fi
    elif [[ $SUBC == *parallel* ]]; then
        echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
        echo "RUN $RUN OLOG $FSCRIPT.log"
    fi
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
