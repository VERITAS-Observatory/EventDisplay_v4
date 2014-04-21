#!/bin/bash
# script to analyse VTS data files with lookup tables
# Author: Gernot Maier

# qsub parameters
h_cpu=00:29:00; h_vmem=2000M; tmpdir_size=4G

if [ $# -lt 3 ]; then
# begin help message
echo "
MSCW_ENERGY data analysis: submit jobs from a simple run list

ANALYSIS.mscw_energy.sh <runlist> <evndisp directory> <table file> [Rec ID]
 [output directory]

required parameters:

    <runlist>               simple run list with one run number per line
    
    <evndisp directory>     directory containing evndisp output ROOT files
    
    <table file>            mscw_energy lookup table file
    
optional parameters:
(use only when processing files for creating radial acceptances)
    
    [Rec ID]                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.
    
    [output directory]      directory where mscw.root files are written
                            default: <evndisp directory>
                            default with Rec ID: <evndisp directory>/RecID#

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
INPUTDIR=$2
TABFILE=$3
TABFILE=${TABFILE%%.root}.root
[[ "$4" ]] && ID=$4 || ID=0
if [[ "$5" ]]; then
    ODIR=$5
elif [[ "$4" ]]; then
    # user passed a Rec ID value
    ODIR=$INPUTDIR/RecID$ID
else
    ODIR=$INPUTDIR
fi

# Read runlist
if [ ! -f "$RLIST" ] ; then
    echo "Error, runlist $RLIST not found, exiting..."
    exit 1
fi
FILES=`cat $RLIST`

# Check that table file exists
if [[ "$TABFILE" == `basename $TABFILE` ]]; then
    TABFILE="$VERITAS_EVNDISP_AUX_DIR/Tables/$TABFILE"
fi
if [ ! -f "$TABFILE" ]; then
    echo "Error, table file not found, exiting..."
    exit 1
fi

# make output directory if it doesn't exist
mkdir -p $ODIR

# run scripts are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.ANADATA"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.mscw_energy_sub"

#########################################
# loop over all files in files loop
for AFILE in $FILES
do
    BFILE="$INPUTDIR/$AFILE.root"
    echo "Now analysing $BFILE (ID=$ID)"

    FSCRIPT="$LOGDIR/MSCW.data-ID$ID-$AFILE"
    rm -f $FSCRIPT.sh

    sed -e "s|TABLEFILE|$TABFILE|" \
        -e "s|RECONSTRUCTIONID|$ID|" \
        -e "s|OUTPUTDIRECTORY|$ODIR|" \
        -e "s|EVNDISPFILE|$BFILE|" $SUBSCRIPT.sh > $FSCRIPT.sh

    chmod u+x $FSCRIPT.sh
    echo $FSCRIPT.sh

    # run locally or on cluster
    SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
    SUBC=`eval "echo \"$SUBC\""`
    if [[ $SUBC == *qsub* ]]; then
		# print the job submission output to stdout, while also copying it to QSUBDATA
        QSUBDATA=$( $SUBC $FSCRIPT.sh | tee >(cat ->&5) )
		
		# get the submitted job's id, after the fact
		# by looking for "Your job ####### ..."
		JOBID=$( echo "$QSUBDATA" | grep -E "Your job" | awk '{ print $3 }' )
		
		# tell the user basic info about the job submission
		echo "RUN$AFILE JOBID $JOBID"
		
		# don't print a .o logfile name if the user specified /dev/null in the qsub command
		if [[ ! $SUBC == */dev/null* ]] ; then
			echo "RUN$AFILE OLOG $FSCRIPT.sh.o$JOBID"
		fi
		echo "RUN$AFILE ELOG $FSCRIPT.sh.e$JOBID"
    elif [[ $SUBC == *parallel* ]]; then
        echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
    fi
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
