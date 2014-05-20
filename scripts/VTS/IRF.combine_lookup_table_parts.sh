#!/bin/bash
# script to combine several table file into one

# qsub parameters
h_cpu=20:29:00; h_vmem=8000M; tmpdir_size=10G

if [[ $# < 4 ]]; then
# begin help message
echo "
IRF generation: create a lookup table from a set of partial table files

IRF.combine_lookup_table_parts.sh <epoch> <atmosphere> <Rec ID> <sim type> [table date] [sim date]

required parameters:

    <epoch>                 array epoch (e.g., V4, V5, V6)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.
                            
    <sim type>              simulation type (e.g. GRISU, CARE)
    
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
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# EventDisplay version
EDVERSION=`$EVNDISPSYS/bin/combineLookupTables --version | tr -d .`

# Parse command line arguments
EPOCH=$1
ATM=$2
RECID=$3
SIMTYPE=$4
[[ "$5" ]] && TABDATE=$5 || TABDATE=`date +"%Y%m%d"`
[[ "$6" ]] && SIMDATE=$6

# input directory containing partial table files
if [[ -n $VERITAS_IRFPRODUCTION_DIR ]]; then
    INDIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/Tables/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}"
else
    INDIR="$VERITAS_USER_DATA_DIR/analysis/$EDVERSION/Tables/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}"
fi
if [[ ! -d $INDIR ]]; then
    echo -e "Error, could not locate input directory. Locations searched:\n $INDIR"
    exit 1
fi
echo "Input file directory: $INDIR"

# Output file directory
if [[ -n $VERITAS_IRFPRODUCTION_DIR ]]; then
    ODIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/Tables/"
else
    ODIR="$VERITAS_USER_DATA_DIR/analysis/$EDVERSION/Tables/"
fi
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR

# Create list of partial table files
FLIST=`ls -1 $INDIR | grep table`

# Obtain table information from partial table names
TESTFILE=FLIST[0]
TESTFILE=${TESTFILE##"table_"}
TESTFILE=${TESTFILE%%'.root'}
SIMTYPE=${TESTFILE%%_*}
PARAMS=V${TESTFILE##*V}
OFILE="table_d${TABDATE}_${SIMTYPE}${SIMDATE}_${PARAMS}"
if [[ -f $OFILE ]]; then
    echo "ERROR: table file $ODIR/$OFILE exists; move it or delete it"
    exit 1
fi

# run scripts and output are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.MAKETABLES"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.lookup_table_combine_sub"

FSCRIPT="$LOGDIR/CMB-TBL.$DATE.MC"
sed -e "s|TABLELIST|$FLIST|" \
    -e "s|OUTPUTFILE|$OFILE|" \
    -e "s|OUTPUTDIR|$ODIR|" $SUBSCRIPT.sh > $FSCRIPT.sh

chmod u+x $FSCRIPT.sh

# run locally or on cluster
SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
SUBC=`eval "echo \"$SUBC\""`
if [[ $SUBC == *qsub* ]]; then
    JOBID=`$SUBC $FSCRIPT.sh`
    echo "JOBID: $JOBID"
elif [[ $SUBC == *parallel* ]]; then
    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
fi

exit
