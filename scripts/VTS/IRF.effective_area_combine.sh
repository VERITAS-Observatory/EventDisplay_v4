#!/bin/bash
# combine many effective area files into one
# Author: Gernot Maier

# qsub parameters
h_cpu=5:29:00; h_vmem=6000M; tmpdir_size=10G

if [ $# -lt 5 ]; then
# begin help message
echo "
IRF generation: combine partial effective area files

IRF.effective_area_combine.sh <input directory> <cuts file> <epoch> <atmosphere>
 <Rec ID> [date]

required parameters:

    <input directory>       directory containing partial effective area files
    
    <cuts file>             gamma/hadron cuts file
        
    <epoch>                 array epoch (e.g., V4, V5, V6)
    
    <atmosphere>            atmosphere model (21 = winter, 22 = summer)
    
    <Rec ID>                reconstruction ID
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.

    optional parameters:
    
    [date]                  date of effective area file generation
                            (default: today's date)
    
examples:

./IRF.effective_area_combine.sh $VERITAS_DATA_DIR/EffectiveAreas/131031
 ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Soft.dat V6 21 0

./IRF.effective_area_combine.sh $VERITAS_DATA_DIR/EffectiveAreas/130601
 ANASUM.GammaHadron.d20131031-cut-N3-Point-005CU-Moderate.dat \"V5 V6\" \"21 22\"
 \"0 1 2 3 4\" 20131115

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
CUTSFILE=$2
EPOCH=$3
ATMOS=$4
RECID=$5
[[ "$6" ]] && EADATE=$6 || EADATE=`date +"%Y%m%d"`

# Run scripts and output are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/EFFAREA"
echo "Writing run scripts and log files to $LOGDIR"
mkdir -p $LOGDIR

# Generate EA base file name based on cuts file
CUTSFILE=`basename $CUTSFILE`
CUTSFILE=${CUTSFILE##ANASUM.GammaHadron.}
CUTSFILE=${CUTSFILE%%.dat}
EANAME="effArea-$CUTSFILE"

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.effective_area_combine_sub"

# loop over all files/cases
for VX in $EPOCH; do
    for ATM in $ATMOS; do
        for ID in $RECID; do
            echo "Processing epoch $VX, atmosphere ATM$ATM, RecID $ID"

            # telescope combinations
            [[ $ID == 0 ]] && T="1234"
            [[ $ID == 1 ]] && T="234"
            [[ $ID == 2 ]] && T="134"
            [[ $ID == 3 ]] && T="124"
            [[ $ID == 4 ]] && T="123"

            # input and output names
            INFILES="$INDIR/*"
            OFILE="$EANAME-ATM$ATM-$VX-T$T-d$EADATE"
            ODIR="$VERITAS_EVNDISP_AUX_DIR/EffectiveAreas"

            FSCRIPT="$LOGDIR/COMB-$CUTSFILE-ATM$ATM-$VX-ID$ID"
            rm -f $FSCRIPT.sh

            sed -e "s|INPUTFILES|$INFILES|" \
                -e "s|OUTPUTFILE|$OFIL|" \
                -e "s|OUTPUTDIR|$ODIR|" $SUBSCRIPT.sh > $FSCRIPT.sh
	    
            chmod u+x $FSCRIPT.sh
            echo $FSCRIPT.sh

            # run locally or on cluster
            SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
            SUBC=`eval "echo \"$SUBC\""`
            if [[ $SUBC == *qsub* ]]; then
                $SUBC $FSCRIPT.sh
            elif [[ $SUBC == *parallel* ]]; then
                echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
            fi
        done
    done
done

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
