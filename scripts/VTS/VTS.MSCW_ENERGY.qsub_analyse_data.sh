#!/bin/tcsh
#
# script to analyse files with lookup tables
#
# Revision $Id: qsub_analyse.sh,v 1.1.2.1.4.1.4.1.6.1.2.4.2.5.2.1.6.2 2011/04/21 06:36:50 gmaier Exp $
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
set LDIR=$DDIR/RecID$RECID/
mkdir -p $LDIR


#################################
# run analysis
cp -v -f $IFIL $ADIR

$EVNDISPSYS/bin/mscw_energy -tablefile $TFIL.root -arrayrecid=$RECID -inputfile $ADIR/$BFIL.root -writeReconstructedEventsOnly=1 > $LDIR/$BFIL.mscw.log

cp -v -f $ADIR/$BFIL.mscw.root $ODIR/
rm -f $ADIR/$BFIL.root

exit

