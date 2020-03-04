#!/bin/bash
# script to analyse files with lookup tables

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

MSCWLOGFILE="$ODIR/$BFILE.mscw.log"
rm -f $MSCWLOGFILE
cp -f -v $INFILE $TEMPDIR

MSCWDATAFILE="$ODIR/$BFILE.mscw.root"

$EVNDISPSYS/bin/mscw_energy         \
    -tablefile $TABFILE             \
    -noshorttree                    \
    -arrayrecid=$RECID              \
    -inputfile $TEMPDIR/$BFILE.root \
    -writeReconstructedEventsOnly=1 &> $MSCWLOGFILE

# move output file from scratch and clean up
cp -f -v $TEMPDIR/$BFILE.mscw.root $MSCWDATAFILE
rm -f $TEMPDIR/$BFILE.mscw.root
rm -f $TEMPDIR/$BFILE.root
    
# write info to log
echo "RUN$BFILE MSCWLOG $MSCWLOGFILE"
echo "RUN$BFILE MSCWDATA $MSCWDATAFILE"

exit
