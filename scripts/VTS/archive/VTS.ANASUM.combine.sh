#!/bin/zsh
# Script to combine ANASUM files ran in parrallel mode

if [ ! -n "$4" ]
then
   echo "VTS.ANASUM.combine <Directory: anasum root files> <ANASUM runlist> <Parameter file> <Output file name (without .root)>"
   exit
fi


INDIR=$1
RUNLIST=$2
PARAMETERFILE=$3
OUTFILE=$4

$EVNDISPSYS/bin/anasum -d $EVNDISPSYS/scripts/VTS/$INDIR -l $EVNDISPSYS/scripts/VTS/$RUNLIST -i 1 -f $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/$PARAMETERFILE -o $OUTFILE.root 2>&1 | tee $OUTFILE.log
