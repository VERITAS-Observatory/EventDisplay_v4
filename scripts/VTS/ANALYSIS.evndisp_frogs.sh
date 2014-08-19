#!/bin/bash
# script to run eventdisplay analysis with FROGS

# qsub parameters
h_cpu=14:29:00; h_vmem=2000M; tmpdir_size=10G

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
EVNDISP data analysis: evndisp FROGS analysis for a simple run list

ANALYSIS.evndisp_frogs.sh <runlist> [output directory] [mscw directory] 
 [pedestals] [VPM]

required parameters:

    <runlist>               simple run list with one run number per line

optional parameters:

    [output directory]      directory where output ROOT files will be stored
    
    [mscw directory]        directory which contains mscw_energy files
    
    [pedestals]             calculate pedestals (default); set to 0 to skip

    [VPM]                   set to 0 to switch off (default is on)

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
[[ "$2" ]] && ODIR=$2    || ODIR="$VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION/"
[[ "$3" ]] && MSCWDIR=$3 || MSCWDIR="$VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION/RecID0/"
[[ "$4" ]] && CALIB=$4   || CALIB=1
[[ "$5" ]] && VPM=$5     || VPM=1

# Read runlist
if [ ! -f "$RLIST" ] ; then
    echo "Error, runlist $RLIST not found, exiting..."
    exit 1
fi
RUNNUMS=`cat $RLIST`

# Log file directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/frogs"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# output directory
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.evndisp_frogs_sub"

SECONDS=`date +"%s"`


# loop over all files in files loop
for RUN in $RUNNUMS; do
    echo "Now starting run $RUN"
    FSCRIPT="$LOGDIR/EVN.data-$RUN"
    
    # get run array epoch using a run info function
    EPOCH=`$EVNDISPSYS/bin/printRunParameter $MSCWDIR/$RUN.mscw.root -epoch`
    if [[ $EPOCH == *error* ]]; then
       echo "error finding array type; does mscw file exist? Skipping run $RUN"
       continue
    fi

    sed -e "s|RUNFILE|$RUN|"             \
        -e "s|CALIBRATIONOPTION|$CALIB|" \
        -e "s|OUTPUTDIRECTORY|$ODIR|"    \
        -e "s|MSCWDIRECTORY|$MSCWDIR|"   \
        -e "s|ARRAYEPOCH|$EPOCH|"        \
        -e "s|USEVPMPOINTING|$VPM|" $SUBSCRIPT.sh > $FSCRIPT.sh

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
        echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.$SECONDS.dat
    elif [[ "$SUBC" == *simple* ]] ; then
    	"$FSCRIPT.sh" |& tee "$FSCRIPT.log"
    fi
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.$SECONDS.dat | $SUBC
fi

exit

