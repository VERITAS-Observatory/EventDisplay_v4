#!/bin/sh
#
# script to analyse data files with anasum
#
# Author: Gernot Maier
#

if [ ! -n "$1" ] && [ ! -n "$2" ]  && [ ! -n "$3" ]  && [ ! -n "$4" ] && [ ! -n "$5" ]
then
   echo "VTS.ANASUM.sub_analyse_data.sh <run list> <data directory> <output directory> <output name> <run parameter file>"
   echo ""
   echo "   <output name> without .root"
   exit
fi

#########################################
# input variables
FLIST=$1
DDIR=$2
ODIR=$3
ONAM=$4
RUNP=$5

# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

#########################################
# directory for run scripts
DATE=`date +"%y%m%d"`
LDIR=$VERITAS_USER_LOG_DIR"/queueAnasum/"$DATE"/"
mkdir -p $LDIR

FSCRIPT="VTS.ANASUM.qsub_analyse_data"

FNAM="$LDIR/ANA.$ONAM"

sed -e "s|FILELIST|$FLIST|" $FSCRIPT.sh > $FNAM-1.sh
sed -e "s|DATADIR|$DDIR|" $FNAM-1.sh > $FNAM-2.sh
rm -f $FNAM-1.sh
sed -e "s|OUTDIR|$ODIR|" $FNAM-2.sh > $FNAM-3.sh
rm -f $FNAM-2.sh
sed -e "s|OUTNAM|$ONAM|" $FNAM-3.sh > $FNAM-4.sh
rm -f $FNAM-3.sh
sed -e "s|RUNPARA|$RUNP|" $FNAM-4.sh > $FNAM.sh
rm -f $FNAM-4.sh

chmod u+x $FNAM.sh
echo $FNAM.sh

qsub -V -l h_cpu=8:00:00 -l h_vmem=4000M -l tmpdir_size=10G -o $LDIR -e $LDIR "$FNAM.sh"

exit
