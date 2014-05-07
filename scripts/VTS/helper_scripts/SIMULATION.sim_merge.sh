#!/bin/bash
# script to run eventdisplay analysis for VTS data

# qsub parameters
h_cpu=41:29:00; h_vmem=2000M; tmpdir_size=10G

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
SIMULATION mergeing: submit jobs to merge multiple vbf files together

SIMULATION.sim_merge.sh <merge tracking file>

required parameters:

    <merge tracking file>   name of text file containing information about
                            merges.  One line per output merged file.
                            Each line should have 4 pieces of information:
        
                            +mergelist=<path to mergelist>
                            +mergeoutput=<output name>
                            +mergelog=<logfile name>
                            +firstcorsika=<first corsika number>
        
merge tracking file parameters:

    <path to mergelist>     merge list should be a text file containing the
                            full path to each input vbf file to be merged,
                            one per line
    
    <output name>           Full path of the final merged file
        
    <logfile name>          Full path of the logfile for the merging process
                            (separate from the batch logfiles)
    
    <first corsika number>  Number of the corsika file to start the merging.
                            i.e. if we want to merge corsika files
                            DAT200224.vbf.bz2 through DAT205000.vbf.bz2,
                            then we use '+firstcorsikajob=200224' .

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

# Parse command line arguments
MERGETRACK=$1

# Read runlist
if [ ! -f "$MERGETRACK" ] ; then
    echo "Error, merge tracking file $MERGETRACK not found, exiting..."
    exit 1
fi

# Output directory for error/output
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/simmer"
mkdir -p $LOGDIR

# job-specific options
MERGETRACKLENGTH=$( cat $MERGETRACK | wc -l )
PARALLELOPT=()
PARALLELOPT+=( -t 1-${MERGETRACKLENGTH}:1 )

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/SIMULATION.sim_merge_sub"

# submit job
DATETIMETAG=$( date +"%Y%m%d-%H%M%S" )
FSCRIPT="$LOGDIR/pip.simmer-$DATETIMETAG"

sed -e "s|MEERRGG|$MERGETRACK|" $SUBSCRIPT.sh > $FSCRIPT.sh

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
    
    # don't print a .o logfile name if the user specified /dev/null in the qsub command
    if [[ ! $SUBC == */dev/null* ]] ; then
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
