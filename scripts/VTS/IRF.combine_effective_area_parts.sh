#!/bin/bash
# combine many effective area files into one

# qsub parameters
h_cpu=5:29:00; h_vmem=6000M; tmpdir_size=10G

if [[ $# < 5 ]]; then
# begin help message
echo "
IRF generation: combine partial effective area files

IRF.combine_effective_area_parts.sh <cuts file> <epoch> <atmosphere> <Rec ID> <sim type> [date]

required parameters:
    
    <cuts file>             gamma/hadron cuts file
        
    <epoch>                 array epoch (e.g., V4, V5, V6)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.
                            
    <sim type>              simulation type (e.g. GRISU, CARE)

optional parameters:

   [name]                   name added to the effective area output file
                            (default is today's date)

examples:

./IRF.combine_effective_area_parts.sh ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft.dat V6 21 0 CARE

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# date
DATE=`date +"%y%m%d"`

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# EventDisplay version
EDVERSION=`$EVNDISPSYS/bin/combineEffectiveAreas --version | tr -d .`

# Parse command line arguments
CUTSFILE=$1
EPOCH=$2
ATMOS=$3
RECID=$4
SIMTYPE=$5
[[ "$6" ]] && EANAME=$6 || EANAME="${DATE}"
PARTICLE_TYPE="gamma"

# Generate EA base file name based on cuts file
CUTS_NAME=`basename $CUTSFILE`
CUTS_NAME=${CUTS_NAME##ANASUM.GammaHadron-}
CUTS_NAME=${CUTS_NAME%%.dat}

# input directory with effective areas
if [[ -n "$VERITAS_IRFPRODUCTION_DIR" ]]; then
    INDIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATMOS}_${PARTICLE_TYPE}/EffectiveAreas_${CUTS_NAME}"
fi
if [[ ! -d $INDIR ]]; then
    echo "Error, could not locate input directory. Locations searched:"
    echo "$INDIR"
    exit 1
fi
INFILES="$INDIR/*ID${RECID}*.root"
echo "Input file directory: $INDIR"
echo "Input files: $INFILES"

# Output file directory
if [[ -n "$VERITAS_IRFPRODUCTION_DIR" ]]; then
    ODIR="$VERITAS_IRFPRODUCTION_DIR/$EDVERSION/$SIMTYPE/${EPOCH}_ATM${ATMOS}_${PARTICLE_TYPE}/EffectiveAreas"
fi
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR

# Run scripts and log files are written into this directory
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EFFAREA"
echo "Writing run scripts and log files to $LOGDIR"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.effective_area_combine_sub"

# loop over all files/cases
echo "Processing epoch $EPOCH, atmosphere ATM$ATMOS, RecID $RECID"

# telescope combinations
[[ $RECID == 0 ]] && T="1234"
[[ $RECID == 1 ]] && T="234"
[[ $RECID == 2 ]] && T="134"
[[ $RECID == 3 ]] && T="124"
[[ $RECID == 4 ]] && T="123"

# output effective area name
OFILE="effArea-$SIMTYPE-${EANAME}-${CUTS_NAME}-${EPOCH}-ATM${ATMOS}-T${T}"

FSCRIPT="$LOGDIR/COMB-EFFAREA-$CUTSFILE-ATM$ATMOS-$EPOCH-ID$RECID"
rm -f $FSCRIPT.sh

sed -e "s|INPUTFILES|$INFILES|" \
    -e "s|OUTPUTFILE|$OFILE|"   \
    -e "s|OUTPUTDIR|$ODIR|" $SUBSCRIPT.sh > $FSCRIPT.sh
	    
chmod u+x $FSCRIPT.sh
echo $FSCRIPT.sh

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