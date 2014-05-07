#!/bin/bash
# script to run eventdisplay analysis with FROGS

# qsub parameters
h_cpu=14:29:00; h_vmem=2000M; tmpdir_size=10G

if [ $# -lt 2 ]; then
# begin help message
echo "
EVNDISP data analysis: evndisp FROGS analysis for a simple run list

ANALYSIS.evndisp_frogs.sh <runlist> <mscw directory> <output directory>
 <array version> [calibration] [VPM]

required parameters:

    <runlist>               simple run list with one run number per line

    <mscw directory>        directory which contains mscw_energy files
    
    <output directory>      directory where output ROOT files will be stored

    <array version>         array version of the runs in the runlist,
                            e.g. V4, V5, V6.  (aka array epoch)

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

# create extra stdout for duplication of command output
# look for ">&5" below
exec 5>&1

# Parse command line arguments
RLIST=$1
MSCWDIR=$2
ODIR=$3
ARRAYVERS=$4
mkdir -p $ODIR
[[ "$5" ]] && CALIB=$5 || CALIB=1
[[ "$6" ]] && VPM=$6   || VPM=1

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

    sed -e "s|RUNFILE|$AFILE|"           \
        -e "s|CALIBRATIONOPTION|$CALIB|" \
        -e "s|OUTPUTDIRECTORY|$ODIR|"    \
        -e "s|MSCWDIRECTORY|$MSCWDIR|"   \
        -e "s|ARRRRRRRAAAYY|$ARRAYVERS|" \
        -e "s|USEVPMPOINTING|$VPM|" $SUBSCRIPT.sh > $FSCRIPT.sh

    chmod u+x $FSCRIPT.sh
    echo $FSCRIPT.sh

    # run locally or on cluster
    SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
    SUBC=`eval "echo \"$SUBC\""`
    if [[ $SUBC == *qsub* ]]; then
        
		# print the job submission output to stdout, while also copying it to QSUBDATA
        QSUBDATA=$( $SUBC $FSCRIPT.sh | tee >(cat - >&5) ) 
        
		# get the submitted job's id, after the fact
		# by looking for "Your job ####### ..."
		JOBID=$( echo "$QSUBDATA" | grep -E "Your job" | awk '{ print $3 }' )
		
		# tell the user basic info about the job submission
		echo "RUN$AFILE JOBID $JOBID"
		
		# don't print a .o logfile name if the user specified /dev/null in the qsub command
		if [[ ! $SUBC == */dev/null* ]] ; then
			echo "RUN$AFILE OLOG $FSCRIPT.sh.o$JOBID"
            echo "RUN$AFILE ELOG $FSCRIPT.sh.e$JOBID"
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

