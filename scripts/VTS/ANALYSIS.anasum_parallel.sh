#!/bin/bash
# script to analyse data files with anasum (parallel analysis)

# qsub parameters
h_cpu=12:29:00; h_vmem=4000M; tmpdir_size=1G

if [[ $# < 2 ]]; then
# begin help message
echo "
ANASUM parallel data analysis: submit jobs from an anasum run list

ANALYSIS.anasum_parallel.sh <anasum run list> <output directory> [run parameter
 file] [mscw directory]

required parameters:

    <anasum run list>       full anasum run list
    
    <output directory>      anasum output files are written to this directory
    
optional parameters:

    [run parameter file]    anasum run parameter file (located in 
                            \$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/;
                            default is ANASUM.runparameter)
    
    [mscw directory]        directory containing the mscw.root files

IMPORTANT! Run ANALYSIS.anasum_combine.sh once all parallel jobs have finished!

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
FLIST=$1
ODIR=$2
[[ "$3" ]] && RUNP=$3  || RUNP="ANASUM.runparameter"
[[ "$4" ]] && INDIR=$4 || INDIR="$VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION/RecID0"

# Check that run list exists
if [[ ! -f "$FLIST" ]]; then
    echo "Error, anasum runlist $FLIST not found, exiting..."
    exit 1
fi

# Check that run parameter file exists
if [[ "$RUNP" == `basename $RUNP` ]]; then
    RUNP="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/$RUNP"
fi
if [[ ! -f "$RUNP" ]]; then
    echo "Error, anasum run parameter file not found, exiting..."
    exit 1
fi

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/ANASUM.ANADATA"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# temporary run list
DATECODE=`date +%Y%m%d`
TEMPLIST=`basename $FLIST`
TEMPLIST="$LOGDIR/$DATECODE.PID$$.$TEMPLIST.tmp"
echo -e "Temporary run list written to:\n $TEMPLIST"
rm -f $TEMPLIST
cat $FLIST | grep "*" >> $TEMPLIST

# output directory
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR
ODIRBASE=`basename $ODIR`
echo -e "Output directory base name:\n $ODIRBASE"

# get list of runs
NLINES=`cat $TEMPLIST | wc -l`
NRUNS=`cat $TEMPLIST | grep -v "VERSION" | wc -l`
echo "Total number of runs to analyse: $NRUNS"

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.anasum_sub"

# loop over all runs
for ((i=1; i <= $NLINES; i++)); do
    LINE=`head -n $i $TEMPLIST | tail -n 1`
    RUN=`head -n $i $TEMPLIST | tail -n 1 | awk '{print $2}'`

    if [[ $RUN != "VERSION" ]]; then
        # output file name
        ONAME="$RUN.anasum"

        # temporary per-run file list
        RUNTEMPLIST="$LOGDIR/qsub_analyse_fileList_${ODIRBASE}_${RUN}_${DATECODE}_PID$$"
        rm -f $RUNTEMPLIST
        echo "$LINE" > $RUNTEMPLIST

        # prepare run scripts
        FSCRIPT="$LOGDIR/qsub_analyse-$DATE-RUN$RUN"
        echo "Run script written to: $FSCRIPT"
        echo "Temporary run list written to: $RUNTEMPLIST"

        sed -e "s|FILELIST|$RUNTEMPLIST|" \
            -e "s|DATADIR|$INDIR|"        \
            -e "s|OUTDIR|$ODIR|"          \
            -e "s|OUTNAME|$ONAME|"        \
            -e "s|RUNNNNN|$RUN|"          \
            -e "s|RUNPARAM|$RUNP|" $SUBSCRIPT.sh > $FSCRIPT.sh

        chmod u+x $FSCRIPT.sh
        
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

rm -f $TEMPLIST

exit
