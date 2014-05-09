#!/bin/bash
# script to run eventdisplay analysis for VTS MC data
#
# qsub parameters
h_cpu=41:29:00; h_vmem=6000M; tmpdir_size=100G


if [ ! -n "$1" ] || [ "$1" = "-h" ]; then
# begin help message
echo "
EVNDISP simulation analysis: submit jobs from a simulation runlist

SIMULATION.evndisp.sh <simulation runlist> 

required parameters:

    <simulation runlist>    formatted list of simulation runs (and options)
                            each simulation run fits on one line (kind of 
                            like the anasum runlist)
                            
In the simulation runlist, each line should have the format:
    +mcfilename=<MC filename> +outputdir=<output directory> +runnumber=<runnumber> +arraycfg=<array config file> [options]

    <MC filename>           full path of the input simulation file (*.vbf or *.vbf.bz2)
    
    <output directory>      directory where output evndisp ROOT files will be stored

    <runnumber>             runnumber to associate with this simulation file
    
    <array config file>     veritas array configuration file, usually something like:
                              EVN_V4_Oct2012_oldArrayConfig_20130428_v420.txt
                              EVN_V5_Oct2012_newArrayConfig_20121027_v420.txt
                              EVN_V6_Upgrade_20121127_v420.txt
                            Warning, should ONLY be the filename, not the full path.
                            Config file is assumed to be in \$VERITAS_EVNDISP_AUX_DIR/DetectorGeometry/ .
    
[options] : optional parameters:
    +mctype=<MC Type> +noisefile=<noise file> +noiselevel=<noise level> 
    
	<MC Type>               specify the program that produced the simulation
                            (Monte Carlo) file
          GRISU             Input simulation file was produced by GRISU (default)
          CARE              Input simulation file was produced by CARE
    
    <noise level>           noise level in GrISU units (default: 200)
	
    <noise file>            location of external GrISU noise file (default:
                            \$VERITAS_EVNDISP_AUX_DIR/NOISE/NOISExxx.grisu
                            where xxx = <noise level>)
                            requires full path
                            will ignore +noiselevel if +noisefile is specified

Within a line, ordering of the arguments/options does not matter, but each argument/option should 
have the format '+optionname=optionarg', separated by spaces.  Spaces are ok in optionarg's.
    
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
MCRUNLIST=$1

if [ ! -f "$MCRUNLIST" ] ; then
    echo "Error, input runlist $MCRUNLIST not found, exiting..."
    exit 1
fi

# Output directory for error/output
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/evndisp_sim"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/SIMULATION.evndisp_sub"


#########################################
# loop over lines in $RUNLIST
while read FILELINE ; do
    echo
    #echo "Now reading '$FILELINE'"
    echo "Reading line..."
    
    # check for badly formatted options
    #   good: '+skfsjf=sekfnsknf +slkfnb=einfaskg'
    #   bad:  '+agnskdfj=slkenfs+eknsf=lsenfsk'
    BADFORMAT=$( echo "$FILELINE" | grep -P "[^ ]\+" ) # match anytime there is a non-space-character before a '+'
    if [[ "$BADFORMAT" ]] ; then
        echo "Error, line '$FILELINE' is badly formatted, make sure there is a space before each '+' sign, exiting..."
        exit 1
    fi

    # split up the line at each '+', for easy grep-ing
    ARGS=$( echo "$FILELINE" | tr '+' '\n')
    
    # extract the arguments from the simrunlist line
    #MCFILE=$(      echo "$ARGS" | grep "mcfilename" | tr '=' ' ' | awk '{ $1=""; print }' | sed -e 's/^ *//' -e 's/ *$//' )
    OUTPUTDIR=$( echo "$ARGS" | grep "outputdir"  | grep -oP "=.+$" | tr -d '=' | sed -e 's/^ *//' -e 's/ *$//' )
    RUNNUM=$(    echo "$ARGS" | grep "runnumber"  | grep -oP "=.+$" | tr -d '=' | sed -e 's/^ *//' -e 's/ *$//' )
    MCTYPE=$(    echo "$ARGS" | grep "mctype"     | grep -oP "=.+$" | tr -d '=' | sed -e 's/^ *//' -e 's/ *$//' )
    NOISELEV=$(  echo "$ARGS" | grep "noiselevel" | grep -oP "=.+$" | tr -d '=' | sed -e 's/^ *//' -e 's/ *$//' )
    NOISEFILE=$( echo "$ARGS" | grep "noisefile"  | grep -oP "=.+$" | tr -d '=' | sed -e 's/^ *//' -e 's/ *$//' )
    MCFILE=$(    echo "$ARGS" | grep "mcfilename" | grep -oP "=.+$" | tr -d '=' | sed -e 's/^ *//' -e 's/ *$//' )
    ARRAYCFG=$(  echo "$ARGS" | grep "arraycfg"   | grep -oP "=.+$" | tr -d '=' | sed -e 's/^ *//' -e 's/ *$//' )
    
    # let the user know which args were read in properly,
    # since misspelled/unrecognized options are quietly ignored
    echo "Recognized the following parameters:"
    [[ "$MCFILE"    ]] && echo "  +mcfilename=$MCFILE"   || ( echo ; echo "Error, must specify '+mcfilename=/path/to/inputfile', exiting..."       ; exit 1 )
    [[ "$OUTPUTDIR" ]] && echo "  +outputdir=$OUTPUTDIR" || ( echo ; echo "Error, must specify '+outputdir=/path/to/output/directory', exiting..." ; exit 1 )
    [[ "$RUNNUM"    ]] && echo "  +runnumber=$RUNNUM"    || ( echo ; echo "Error, must specify '+runnumber=#####', exiting..."                     ; exit 1 )
    [[ "$ARRAYCFG"  ]] && echo "  +arraycfg=$ARRAYCFG"   || ( echo ; echo "Error, must specify '+arraycfg=nameOfConfigFile', exiting..."  ; exit 1 )
    [[ "$MCTYPE"    ]] && echo "  +mctype=$MCTYPE"       || MCTYPE="GRISU"
    [[ "$NOISELEV"  ]] && echo "  +noiselevel=$NOISELEV" || NOISE="200"
    [[ "$NOISEFILE" ]] && echo "  +noisefile=$NOISEFILE" || NOISEFILE="$VERITAS_EVNDISP_AUX_DIR/NOISE/NOISE$NOISELEV.grisu"
    echo "  these are the ONLY options detected, any mispelled or unrecognized options are ignored!"

    # check the main args
    if [ ! -f "$MCFILE" ] ; then
        echo ; echo "Error (with option '+mcfilename=$MCFILE'), input simulation file not found, exiting..."
        exit 1
    elif [[ "$MCFILE" != *.vbf && "$MCFILE" != *.vbf.bz2 ]] ; then
        echo ; echo "Error with option '+mcfilename=$MCFILE', input simulation file must end in either .vbf or .vbf.bz2, exiting..."
        exit 1
    fi
    
    # check MC type
    if [[ ! "$MCTYPE" =~ (GRISU|CARE) ]] ; then
        echo ; echo "Error with '+mctype=$MCTYPE' must be either 'GRISU' or 'CARE', exiting..."
        exit 1
    elif [[ "$MCTYPE" =~ CARE ]] ; then
        echo ; echo "Error with '+mctype=$MCTYPE', CARE support hasn't been added to this script yet, exiting..."
        exit 1
    fi
    
    # check for valid noise level/file
    if [[ ! -f "$NOISEFILE" ]] ; then
        echo ; echo "Error, noise file $NOISEFILE (from either +noiselevel or +noisefile) doesn't exist in \$VERITAS_EVNDISP_AUX_DIR/NOISE , exiting..."
        exit 1
    fi

    echo "Now starting submitting simulation run $RUNNUM using file $MCFILE"
    FSCRIPT="$LOGDIR/EVN.SIM-$RUNNUM"
    echo "FSCRIPT: $FSCRIPT.sh"

    sed -e "s|INPUTFILEINPUTFILE|$MCFILE|"     \
        -e "s|RUNRUNRUN|$RUNNUM|"              \
        -e "s|OUTPUTDIROUTPUTDIR|$OUTPUTDIR|"  \
        -e "s|NOISENOISE|$NOISE|"              \
        -e "s|NOISEFILENOISEFILE|$NOISEFILE|"  \
        -e "s|CONFIGCONFIG|$ARRAYCFG|"         \
        $SUBSCRIPT.sh > $FSCRIPT.sh
    
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
		echo "RUN$RUNNUM JOBID $JOBID"
		
		# don't print a .o logfile name if the user specified /dev/null in the qsub command
		if [[ ! $SUBC == */dev/null* ]] ; then
			echo "RUN$RUNNUM OLOG $FSCRIPT.sh.o$JOBID"
		fi
		echo "RUN$RUNNUM ELOG $FSCRIPT.sh.e$JOBID"

    elif [[ $SUBC == *parallel* ]]; then
        echo "$FSCRIPT.sh &> $FSCRIPT.log" >> $LOGDIR/runscripts.dat
    fi

done < "$MCRUNLIST"

# Execute all FSCRIPTs locally in parallel
if [[ $SUBC == *parallel* ]]; then
    cat $LOGDIR/runscripts.dat | $SUBC
fi

exit
