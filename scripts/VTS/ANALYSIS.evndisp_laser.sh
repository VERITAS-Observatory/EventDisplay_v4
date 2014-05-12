#!/bin/bash
# script run eventdisplay laser analysis with a queue system

# qsub parameters
h_cpu=11:29:00; h_vmem=2000M; tmpdir_size=5G

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
EVNDISP laser analysis: submit jobs from a simple run list

ANALYSIS.evndisp_laser.sh <runlist>

required parameters:

    <runlist>           simple run list with one run number per line

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
RLIST=$1

# Read runlist
if [[ ! -f "$RLIST" ]] ; then
    echo "Error, runlist $RLIST not found, exiting..."
    exit 1
fi
RUNNUMS=`cat $RLIST`
echo "Laser files to analyze:"
echo $RUNNUMS

# Output directory for error/output from batch system
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EVNDISP.ANADATA"
echo "Log files will be written to: $LOGDIR"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.evndisp_laser_sub"

#########################################
# loop over all files in files loop
for RUN in $RUNNUMS; do
    # check if laser file exists
    DFILE=`find -L $VERITAS_DATA_DIR/data/ -name "$RUN.cvbf"`
    if [ -z "$DFILE" ]; then
        echo "Error: laser vbf file not found for run $RUN"
    else
        echo "Now starting laser run $RUN"
        FSCRIPT="$LOGDIR/EVN.laser-$RUN"

        sed -e "s|RUNFILE|$RUN|" \
            -e "s|LOGDIRECTORY|$LOGDIR|" $SUBSCRIPT.sh > $FSCRIPT.sh

        chmod u+x $FSCRIPT.sh
        echo $FSCRIPT.sh

        # run locally or on cluster
        SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
        SUBC=`eval "echo \"$SUBC\""`
        if [[ $SUBC == *qsub* ]]; then
            JOBID=`$SUBC $FSCRIPT.sh`
            echo "RUN $RUN: JOBID $JOBID"
        elif [[ $SUBC == *parallel* ]]; then
            echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
        fi
    fi
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
