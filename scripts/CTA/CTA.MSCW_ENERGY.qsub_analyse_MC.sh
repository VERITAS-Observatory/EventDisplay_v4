#$ -S /bin/tcsh
#
# script to analyse CTA MC files with lookup tables
#
# Author: Gernot Maier
#

set TABFIL=TABLEFILE
set RECID=RECONSTRUCTIONID
set IFIL="IIIIFIL"
set TFIL=TTTTFIL
set ARRAY=ARRAYYY
set DSET=DATASET
set ADIR=AAAAADIR

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

# output data and log files are written to this directory
set ODIR=$CTA_USER_DATA_DIR"/analysis/AnalysisData/"$DSET"/"$ARRAY"/$ADIR/"
mkdir -p $ODIR

# delete old log files
rm -f $ODIR/$TFIL.log

###############################################
# mscw_energy command line options
###############################################
set MOPT="-pe -arrayrecid=$RECID -noNoTrigger -writeReconstructedEventsOnly -shorttree"

# analyse MC file
$EVNDISPSYS/bin/mscw_energy $MOPT -tablefile $TABFIL-$ARRAY.root -inputfilelist $IFIL -outputfile $ODIR/$TFIL.root >& $ODIR/$TFIL.log

# sleep
sleep 2

exit
