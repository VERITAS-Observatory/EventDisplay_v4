#!/bin/bash
# script to run eventdisplay analysis for VTS data
# Author: Gernot Maier
h_cpu=00:29:00 ; h_vmem=3G ; tmpdir_size=20G

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
SIMULATION grisudet analysis: submit jobs from complex runlist

SIMULATION.grisudet.sh <complexrunlist>
    
--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# create extra stdout for duplication of command output
# look for ">&5" below
exec 5>&1

RLIST=$1
SAVELOG=$2
# Read runlist
if [ ! -f "$RLIST" ] ; then
    echo "Error, runlist $RLIST not found, exiting..."
    exit 1
fi


SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/SIMULATION.detectorsim_sub"
echo "Script base: $SUBSCRIPT"

LOGDIR="$VERITAS_USER_LOG_DIR/simulations/detectorsim/"
mkdir -p $LOGDIR

SAVELOGOPT=()
if [[ "$SAVELOG" == "dosavelog" ]] ; then
    # save log files to one 
    SAVELOGOPT+=( -j y -o $LOGDIR/ )
fi

RLISTLENGTH=$( cat $RLIST | wc -l )
PARALLELOPT=()
PARALLELOPT+=( -t 1-${RLISTLENGTH}:1 )

DATETIMETAG=$( date +"%Y%m%d-%H%M%S" )
FSCRIPT="$LOGDIR/pip.detsim-$DATETIMETAG"
sed -e "s|TRACKTRACKTRACK|$RLIST|" $SUBSCRIPT.sh > $FSCRIPT.sh

chmod u+x $FSCRIPT.sh
echo $FSCRIPT.sh

# run locally or on cluster
SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
SUBC=`eval "echo \"$SUBC\""`
if [[ $SUBC == *qsub* ]]; then
    
    # print the job submission output to stdout, while also copying it to QSUBDATA
    QSUBDATA=$( $SUBC ${PARALLELOPT[@]} ${SAVELOGOPT[@]} $FSCRIPT.sh | tee >(cat - >&5) ) 
    
    # get the submitted job's id, after the fact
    # by looking for "Your job ####### ..."
    JOBID=$( echo "$QSUBDATA" | grep -E "Your job" | awk '{ print $3 }' )
    
    # tell the user basic info about the job submission
    echo "JOBID $JOBID"
    echo "SUBSCRIPT $FSCRIPT.sh"
    
    if [[ "$SAVELOG" == "dosavelog" ]] ; then
        echo "BATCHLOG $FSCRIPT.sh.o$JOBID"
    fi

elif [[ $SUBC == *parallel* ]]; then
    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat

fi

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit





