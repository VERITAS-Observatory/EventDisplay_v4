#!/bin/tcsh
#
# script to analyse MC files with lookup tables
#
# Revision $Id: qsub_analyse_MC_CTA.sh,v 1.1.2.1.2.1.2.1 2011/02/14 08:09:53 gmaier Exp $
#
# Author: Gernot Maier
#

set TABFIL=TABLEFILE
set RECID=RECONSTRUCTIONID
set IFIL="IIIIFIL"
set TFIL=TTTTFIL
set ARRAY=ARRAYYY
set DSET=DATASET

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

# output data files are written to this directory
set ODIR=$CTA_USER_DATA_DIR"/analysis/AnalysisData/"$DSET"/"$ARRAY"/Analysis/"
mkdir -p $ODIR

# output log files are written to this directory
set LDIR=$CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET"/"$ARRAY"/Analysis/"
mkdir -p $LDIR

# delete old log files
rm -f $LDIR/$TFIL.log

###############################################
# mscw_energy command line options
###############################################

set MOPT="-pe -arrayrecid=$RECID -noNoTrigger -writeReconstructedEventsOnly -shorttree"
#set MOPT="-pe -arrayrecid=$RECID -noNoTrigger -writeReconstructedEventsOnly"

# analyse MC file
$EVNDISPSYS/bin/mscw_energy $MOPT -tablefile $TABFIL-$ARRAY.root -inputfile "$IFIL*.root" -outputfile $ODIR/$TFIL.root >& $LDIR/$TFIL.log

# sleep
sleep 2

exit
