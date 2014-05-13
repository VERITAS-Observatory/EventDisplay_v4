#!/bin/bash
# script to analyse VTS data files with lookup tables

# qsub parameters
h_cpu=00:29:00; h_vmem=2000M; tmpdir_size=4G

if [[ $# < 2 ]]; then
# begin help message
echo "
MSCW_ENERGY data analysis: submit jobs from a simple run list

ANALYSIS.mscw_energy.sh <runlist> <evndisp directory> <table file> [Rec ID]
 [output directory]

required parameters:

    <runlist>               simple run list with one run number per line
    
    <table file>            mscw_energy lookup table file (located in 
                             \$VERITAS_EVNDISP_AUX_DIR/Tables)
    
optional parameters:
    
    [Rec ID]                reconstruction ID
                            Set to 0 for all telescopes, 1 to cut T1, etc.
                            (use only when making radial acceptances)
                            
    [evndisp directory]     directory containing evndisp output ROOT files
    
    [output directory]      directory where mscw.root files are written
                            default: [evndisp directory]/RecID#

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
TABFILE=$2
TABFILE=${TABFILE%%.root}.root
[[ "$3" ]] && ID=$3    || ID=0
[[ "$4" ]] && INDIR=$4 || INDIR="$VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION/"
[[ "$5" ]] && ODIR=$5  || ODIR="$VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION/RecID$ID"

# Read runlist
if [[ ! -f "$RLIST" ]]; then
    echo "Error, runlist $RLIST not found, exiting..."
    exit 1
fi
RUNNUMS=`cat $RLIST`

# Check that table file exists
if [[ "$TABFILE" == `basename $TABFILE` ]]; then
    TABFILE="$VERITAS_EVNDISP_AUX_DIR/Tables/$TABFILE"
fi
if [[ ! -f "$TABFILE" ]]; then
    echo "Error, table file not found, exiting..."
    exit 1
fi

# make output directory if it doesn't exist
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR

# run scripts are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.ANADATA"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.mscw_energy_sub"

#########################################
# loop over all files in files loop
for RUN in $RUNNUMS; do
    RUNFILE="$INDIR/$RUN.root"
    echo "Now analysing $RUNFILE (ID=$ID)"

    FSCRIPT="$LOGDIR/MSCW.data-ID$ID-$RUN"
    rm -f $FSCRIPT.sh

    sed -e "s|TABLEFILE|$TABFILE|" \
        -e "s|RECONSTRUCTIONID|$ID|" \
        -e "s|OUTPUTDIRECTORY|$ODIR|" \
        -e "s|EVNDISPFILE|$RUNFILE|" $SUBSCRIPT.sh > $FSCRIPT.sh

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
    fi
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
