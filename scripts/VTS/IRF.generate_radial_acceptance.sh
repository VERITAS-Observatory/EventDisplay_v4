#!/bin/bash
# calculate radial acceptances

# qsub parameters
h_cpu=00:29:00; h_vmem=6000M; tmpdir_size=10G

if [[ $# != 5 ]]; then
# begin help message
echo "
IRF generation: create radial acceptances for a set of cuts

IRF.generate_radial_acceptance.sh <runlist> <input directory> <cuts list file> <epoch> <Rec ID>

required parameters:

    <runlist>               simple format run list with one run number per line

    <input directory>       directory containing mscw_energy ROOT files from a
                            gamma-ray dark region of the sky (data, not MC)
        
    <cuts list file>        file containing one gamma/hadron cuts file per line
        
    <epoch>                 array epoch(s) (e.g., V4, V5, V6)
                            V4: array before T1 move (before Fall 2009)
                            V5: array after T1 move (Fall 2009 - Fall 2012)
                            V6: array after camera update (after Fall 2012)
    
    <Rec ID>                reconstruction ID(s)
                            (see EVNDISP.reconstruction.runparameter)
                            Set to 0 for all telescopes, 1 to cut T1, etc.

(note: hardwired reconstruction IDs vs telescope combinations)

examples:
./IRF.generate_radial_acceptance.sh ~/runlist.dat \
 \$VERITAS_USER_DATA_DIR/rad_accept ~/cutset.dat V6 0
    
./IRF.generate_radial_acceptance.sh ~/runlist.dat \
 \$VERITAS_USER_DATA_DIR/rad_accept ~/cutset.dat \"V4 V5 V6\" \"0 1 2 3 4\"
    
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
MSCWDIR=$2
CUTLISTFILE=$3
EPOCH=$4
RECID=$5
# make radial acceptance version
EDVERSION=`$EVNDISPSYS/bin/makeRadialAcceptance --version | tr -d .`
# version string for aux files
AUX="auxv01"

# Read runlist
if [[ ! -f "$RLIST" ]]; then
    echo "Error, runlist $RLIST not found, exiting..."
    exit 1
fi
FILES=`cat $RLIST`

# Read cuts list file
if [[ ! -f "$CUTLISTFILE" ]]; then
    echo "Error, cuts list file not found, exiting..."
    echo $CUTLISTFILE
    exit 1
fi
IFS=$'\r\n' CUTLIST=($(cat $CUTLISTFILE))

# run scripts and logs are written into this directory
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/RADIAL"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/IRF.radial_acceptance_sub.sh"

# loop over all files/cases
for CUTS in ${CUTLIST[@]}; do
    for VX in $EPOCH; do
        for ID in $RECID; do
            echo "Now generating radial acceptance for $CUTS, epoch $VX, Rec ID $ID"

            # telescope combinations
            [[ $ID == "0" ]] && TELES="1234"
            [[ $ID == "2" ]] && TELES="234"
            [[ $ID == "3" ]] && TELES="134"
            [[ $ID == "4" ]] && TELES="124"
            [[ $ID == "5" ]] && TELES="123"
            [[ $ID == "7" ]] && TELES="12"
            [[ $ID == "1" ]] && TELES="1234"
            [[ $ID == "7" ]] && TELES="234"
            [[ $ID == "8" ]] && TELES="134"
            [[ $ID == "9" ]] && TELES="124"
            [[ $ID == "10" ]] && TELES="123"

            # Check that cuts file exists
            CUTSNAME=${CUTS%%.dat}.dat
            if [[ "$CUTSNAME" == `basename $CUTSNAME` ]]; then
                CUTSFILE="$VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles/$CUTSNAME"
            fi
            if [[ ! -f "$CUTSFILE" ]]; then
                echo "Error, gamma/hadron cuts file not found, exiting..."
                echo "cut file $CUTSFILE"
                exit 1
            fi
            
			# Used Method (GEO or DISP)
			if [[ $RECID == "0" || $RECID == "2" || $RECID == "3" || $RECID == "4" || $RECID == "5" || $RECID == "6" ]];then
				METH = "GEO"
			elif [[ $RECID == "1" || $RECID == "7" || $RECID == "8" || $RECID == "9" || $RECID == "10" ]]; then 
				METH = "DISP"
			fi

            # Generate base file name based on cuts file
            CUTSNAME=${CUTSNAME##ANASUM.GammaHadron-}
            CUTSNAME=${CUTSNAME%%.dat}
            #OFILE="radialAcceptance-${EDVERSION}-${AUX}-$CUTSNAME-ID${RECID}-$VX-T$TELES"
			OFILE="radialAcceptance-${EDVERSION}-${AUX}-$CUTSNAME-${METH}-$VX-T$TELES"
            ODIR="$VERITAS_IRFPRODUCTION_DIR/RadialAcceptances"
            mkdir -p $ODIR
			chmod -R g+w $ODIR
            echo -e "Output files will be written to:\n$ODIR"
            echo "Output file name $OFILE"
            
            FSCRIPT="$LOGDIR/RADIAL-$CUTSNAME-$VX-$ID"
            sed -e "s|RUNLIST|$RLIST|"     \
                -e "s|INPUTDIR|$MSCWDIR|"  \
                -e "s|CUTSFILE|$CUTSFILE|" \
                -e "s|OUTPUTDIR|$ODIR|"    \
                -e "s|IEPO|$VX|" \
                -e "s|OUTPUTFILE|$OFILE|" $SUBSCRIPT > $FSCRIPT.sh
            
            chmod u+x $FSCRIPT.sh
            echo "Script submitted to cluster: $FSCRIPT.sh"
            
            # run locally or on cluster
            SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
            echo "$SUBC"
            echo $LOGDIR
            SUBC=`eval "echo \"$SUBC\""`
            if [[ $SUBC == *qsub* ]]; then
                echo $FSCRIPT.sh
#                JOBID=`$SUBC $FSCRIPT.sh`
                #### (GM) for some odd reason the normal way of getting the submission command does not work ####
                JOBID=`qsub -P cta_high -js 10000 -V -terse -l os=sl6 -l h_cpu=00:29:00 -l h_vmem=6000M -l tmpdir_size=10G -o $LOGDIR -e $LOGDIR $FSCRIPT.sh`
                echo "JOBID: $JOBID"
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
