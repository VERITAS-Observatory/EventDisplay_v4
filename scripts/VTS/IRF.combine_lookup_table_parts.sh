#!/bin/bash
# script to combine several table file into one
# Author: Gernot Maier

# qsub parameters
h_cpu=20:29:00; h_vmem=8000M; tmpdir_size=10G

if [ $# -lt 4 ]; then
# begin help message
echo "
IRF generation: create a lookup table from a set of partial table files

IRF.combine_lookup_table_parts.sh <epoch> <atmosphere> <Rec ID> <sim type>
 [table date] [sim date]

required parameters:

    <epoch>                 array epoch (e.g., V4, V5, V6)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.
                            
    <sim type>              original VBF file simulation type (e.g. GRISU, CARE)
    
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

# input directory containing partial table files
if [[ -z $VERITAS_IRF_ANA_DIR ]]; then
    INDIR="$VERITAS_IRF_ANA_DIR/$EDVERSION/Tables/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}"
elif [[ ! -z $VERITAS_IRF_ANA_DIR || ! -d $INDIR ]]; then
    INDIR="$VERITAS_DATA_DIR/analysis/$EDVERSION/Tables/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}"
elif [[ ! -d $INDIR ]]; then
    echo "Error, could not locate input directory. Locations searched:"
    echo "$VERITAS_IRF_ANA_DIR/$EDVERSION/Tables/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}"
    echo "$VERITAS_DATA_DIR/analysis/$EDVERSION/Tables/${SIMTYPE}/${EPOCH}_ATM${ATM}_ID${RECID}"
    exit 1
fi
echo "Input file directory: $INDIR"

# Output file directory
if [[ -z $VERITAS_IRF_ANA_DIR ]]; then
    ODIR="$VERITAS_IRF_ANA_DIR/$EDVERSION/Tables/"
else
    ODIR="$VERITAS_DATA_DIR/analysis/$EDVERSION/Tables/"
fi
echo "Output file directory: $ODIR"
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
if [[ -e $OFILE ]]; then
    echo "ERROR: table file $ODIR/$OFILE exists; move it or delete it"
    exit 1
fi

# run scripts and output are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/MSCW.MAKETABLES"
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
    $SUBC $FSCRIPT.sh
elif [[ $SUBC == *parallel* ]]; then
    echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
fi

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
