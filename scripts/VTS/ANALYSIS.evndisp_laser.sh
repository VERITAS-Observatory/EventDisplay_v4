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
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
RLIST=$1

# Read runlist
if [ ! -f "$RLIST" ] ; then
    echo "Error, runlist $RLIST not found, exiting..."
    exit 1
fi
FILES=`cat $RLIST`
echo "Laser files to analyze:"
echo $FILES

# Output directory for error/output from batch system
# if submitting a lot of scripts, use LOGDIR=/dev/null
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EVNDISP.ANADATA"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.evndisp_laser_sub"

#########################################
# loop over all files in files loop
for AFILE in $FILES
do
    # check if laser file exists
    DFILE=`find -L $VERITAS_DATA_DIR/data/ -name "$AFILE.cvbf"`
    if [ -z "$DFILE" ]; then
        echo "Error: laser vbf file not found for run $AFILE"
    else
        echo "Now starting laser run $AFILE"
        FSCRIPT="$LOGDIR/EVN.laser-$AFILE"

        sed -e "s|RUNFILE|$AFILE|" \
            -e "s|LOGDIRECTORY|$LOGDIR|" $SUBSCRIPT.sh > $FSCRIPT.sh

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
    fi
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
