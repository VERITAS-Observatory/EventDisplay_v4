#!/bin/bash
# script to combine several table file into one
# Author: Gernot Maier

# qsub parameters
h_cpu=20:29:00; h_vmem=8000M; tmpdir_size=10G

if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
IRF generation: create a lookup table from a set of partial table files

IRF.table_combine.sh <input directory> [table date] [sim date]

required parameters:

    <input directory>       directory containing partial table ROOT files
    
optional parameters:

    [table date]            table file creation date (e.g. 20131121)
                            (default: today's date)
    
    [sim date]              simulation creation date (e.g. Nov12)
                            (default: blank)

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
INDIR=$1
[[ "$2" ]] && TABDATE=$2 || TABDATE=`date +"%Y%m%d"`
[[ "$3" ]] && SIMDATE=$3 || SIMDATE=""

# Create list of partial table files
FLIST=`ls -1 $INDIR | grep table`

# run scripts and output are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.MAKETABLES"
mkdir -p $LOGDIR

# Obtain table information from partial table names
TESTFILE=FLIST[0]
TESTFILE=${TESTFILE##"table_"}
TESTFILE=${TESTFILE%%'.root'}
SIMTYPE=${TESTFILE%%_*}
PARAMS=V${TESTFILE##*V}

# output file name and location
OFILE="table_d${TABDATE}_${SIMTYPE}${SIMDATE}_${PARAMS}"
ODIR="$VERITAS_EVNDISP_AUX_DIR/Tables"

if [[ -e $OFILE ]]; then
    echo "ERROR: table file $ODIR/$OFILE exists; move it or delete it"
    exit 1
fi

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.table_combine_sub"

FSCRIPT="$LOGDIR/CMB-TBL.$DATE.MC"
sed -e "s|TABLELIST|$FLIST|" \
    -e "s|OUTPUTFILE|$OFILE|" \
    -e "s|OUTPUTDIR|$ODIR|" $SUBSCRIPT.sh > $FSCRIPT.sh

chmod u+x $FSCRIPT.sh

# run locally or on cluster
SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
SUBC=`eval "echo \"$SUBC\""`
if [[ $SUBC == *qsub* ]]; then
    $SUBC $FSCRIPT.sh
elif [[ $SUBC == *parallel* ]]; then
    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
fi

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
