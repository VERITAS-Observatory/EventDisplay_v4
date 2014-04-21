#!/bin/bash
# script to analyse files with lookup tables
# Author: Gernot Maier

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
TABFILE=TABLEFILE
RECID=RECONSTRUCTIONID
ODIR=OUTPUTDIRECTORY
INFILE=EVNDISPFILE

INDIR=`dirname $INFILE`
BFILE=`basename $INFILE .root`

# temporary (scratch) directory
if [[ -n $TMPDIR ]]; then
    TEMPDIR=$TMPDIR
else
    TEMPDIR="$VERITAS_USER_DATA_DIR/TMPDIR"
fi
mkdir -p $TEMPDIR

#################################
# run analysis
rm -f $ODIR/$BFILE.mscw.log
cp -f -v $INFILE $TEMPDIR

$EVNDISPSYS/bin/mscw_energy -tablefile $TABFILE -arrayrecid=$RECID -inputfile $TEMPDIR/$BFILE.root -writeReconstructedEventsOnly=1 &> $ODIR/$BFILE.mscw.log

# move output file from scratch and clean up
cp -f -v $TEMPDIR/$BFILE.mscw.root $ODIR/$BFILE.mscw.root
rm -f $TEMPDIR/$BFILE.mscw.root
rm -f $TEMPDIR/$BFILE.root

# write info to log
echo "RUN$BFILE MSCWLOG $ODIR/$BFILE.mscw.log"
echo "RUN$BFILE DATAOUT $ODIR/$BFILE.mscw.root"

exit
