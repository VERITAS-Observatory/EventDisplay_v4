#$ -S /bin/tcsh
#
# script to analyse files with lookup tables
#
# Author: Gernot Maier
#

set TFIL=TABLEFILE
set RECID=RECONSTRUCTIONID
set IFIL=EVNDFIL

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh VERITAS

set DDIR=`dirname $IFIL`
set BFIL=`basename $IFIL .root`

# temporary directory
set ADIR=$TMPDIR/RecID$RECID/
mkdir -p $ADIR

# output data files are written to this directory
set ODIR=$DDIR/RecID$RECID/
mkdir -p $ODIR
chmod -R g+w $ODIR

# output log files are written to this directory
set LDIR=$ODIR
rm -f $LDIR/$BFIL.mscw.log

#################################
# run analysis
cp -v -f $IFIL $ADIR

$EVNDISPSYS/bin/mscw_energy -tablefile $TFIL.root -arrayrecid=$RECID -inputfile $ADIR/$BFIL.root -writeReconstructedEventsOnly=1 > $LDIR/$BFIL.mscw.log
echo "RUN$BFIL MSCWLOG $LDIR/$BFIL.mscw.log"

cp -v -f $ADIR/$BFIL.mscw.root $ODIR/
echo "RUN$BFIL DATAOUT $ODIR/$BFIL.mscw.root"
rm -f $ADIR/$BFIL.root

exit

