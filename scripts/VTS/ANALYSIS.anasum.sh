#!/bin/bash
# script to analyse data files with anasum
# Author: Gernot Maier

# qsub parameters
h_cpu=8:00:00; h_vmem=4000M; tmpdir_size=10G

if [ $# -ne 5 ]; then
# begin help message
echo "
ANASUM data analysis: submit jobs from an anasum run list

ANALYSIS.anasum.sh <anasum run list> <mscw directory> <output directory>
 <output file name> <run parameter file>

required parameters:

    <anasum run list>       full anasum run list
    
    <mscw directory>        directory containing the mscw.root files
    
    <output directory>      anasum output files are written to this directory
    
    <output file name>      name of combined anasum file
                            (written to same location as anasum files)
    
    <run parameter file>    anasum run parameter file
                            (in \$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/;
                             see ANASUM.runparameter.dat for an example)

--------------------------------------------------------------------------------
"
#end help message
exit
fi

# Run init script
bash "$( cd "$( dirname "$0" )" && pwd )/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
FLIST=$1
INDIR=$2
ODIR=$3
ONAME=$4
ONAME=${ONAME%%.root}
RUNP=$5

# Check that run list exists
if [ ! -f "$FLIST" ]; then
    echo "Error, anasum runlist $FLIST not found, exiting..."
    exit 1
fi

# Check that run parameter file exists
if [[ "$RUNP" == `basename $RUNP` ]]; then
    RUNP="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/$RUNP"
fi
if [ ! -f "$RUNP" ]; then
    echo "Error, anasum run parameter file not found, exiting..."
    exit 1
fi

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/ANASUM.ANADATA"
mkdir -p $LOGDIR

# Job submission script
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/ANALYSIS.anasum_sub"

FSCRIPT="$LOGDIR/ANA.$ONAME"
sed -e "s|FILELIST|$FLIST|" \
    -e "s|DATADIR|$INDIR|"  \
    -e "s|OUTDIR|$ODIR|"    \
    -e "s|OUTNAME|$ONAME|"  \
    -e "s|RUNPARAM|$RUNP|" $SUBSCRIPT.sh > $FSCRIPT.sh

chmod u+x $FSCRIPT.sh
echo $FSCRIPT.sh

# run locally or on cluster
SUBC=`$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.readSubmissionCommand.sh`
if [[ $SUBC == *qsub* ]]; then
    SUBC=`eval "echo \"$SUBC\""`
    $SUBC $FSCRIPT.sh
else
    $FSCRIPT.sh &> $FSCRIPT.log
fi

exit
