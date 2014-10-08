#!/bin/bash
# script to analyse files with lookup tables

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
TABFILE=TABLEFILE
RECID=RECONSTRUCTIONID
ODIR=OUTPUTDIRECTORY
INFILE=EVNDISPFILE
ENERGY3D=USEENERGY3D

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

echo
echo "\$ENERGY3D:'$ENERGY3D'"
echo

if [ "$ENERGY3D" == "no" ] ; then

    echo "\$ENERGY3D = no :("
    $EVNDISPSYS/bin/mscw_energy         \
        -tablefile $TABFILE             \
        -noshorttree                    \
        -arrayrecid=$RECID              \
        -inputfile $TEMPDIR/$BFILE.root \
        -writeReconstructedEventsOnly=1 \
        |& tee $MSCWLOGFILE

    # move output file from scratch and clean up
    cp -f -v $TEMPDIR/$BFILE.mscw.root $MSCWDATAFILE
    rm -f $TEMPDIR/$BFILE.mscw.root
    rm -f $TEMPDIR/$BFILE.root
    
# nils' energy3d step
elif [ "$ENERGY3D" == "yes" ] ; then
    
    echo "\$ENERGY3D = yes!"
    OUTPUTFILE="$BFILE.energy3d"    
    
    EPOCH=$(     $EVNDISPSYS/bin/printRunParameter $TEMPDIR/$BFILE.root -epoch      )
    ATMO=$(      $EVNDISPSYS/bin/printRunParameter $TEMPDIR/$BFILE.root -atmosphere )
    TELTOANA=$(  $EVNDISPSYS/bin/printRunParameter $TEMPDIR/$BFILE.root -teltoana   )
    
    TEMPLATEFNAME="Template3D_V${EPOCH}_ATM${ATMO}_${TELTOANA}.root"
    cp $VERITAS_EVNDISP_AUX_DIR/Energy3DTemplates/Merged/$TEMPLATEFNAME $TEMPDIR/$TEMPLATEFNAME

    $EVNDISPSYS/bin/energy3d \
        -inputfile   $TEMPDIR/$BFILE.root      \
        -outputfile  $TEMPDIR/$OUTPUTFILE.root \
        -template    $TEMPDIR/$TEMPLATEFNAME   \
        |& tee $MCWLOGFILE
    
    echo
    echo "Here's whats in the temporary directory!"
    ls -l $TEMPDIR/
    echo

    cp -f -v $TEMPDIR/$OUTPUTFILE.root $MSCWDATAFILE
    rm -f $TEMPDIR/$BFILE.mscw.root
    rm -f $TEMPDIR/$BFILE.root
    rm -f $TEMPDIR/$TEMPLATEFNAME
    
fi

# write info to log
echo "RUN$BFILE MSCWLOG $MSCWLOGFILE"
echo "RUN$BFILE MSCWDATA $MSCWDATAFILE"

exit
