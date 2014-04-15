#$ -S /bin/tcsh
#
# make tables for CTA
#
#
# Author: Gernot Maier
#

set TFIL=TABLEFILE
set RECID=RECONSTRUCTIONID
set ARRAY=ARRRRRRR
set DDIR="DATADIRECT"
set DSET="DATASET"
set SETOFF="CTAOFF"

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

# output data files are written to this directory
set ODIR=$CTA_USER_DATA_DIR"/analysis/AnalysisData/$DSET/"$ARRAY"/Tables/"
mkdir -p $ODIR

# output log files are written to this directory
set LDIR=$CTA_USER_DATA_DIR"/analysis/AnalysisData/$DSET/"$ARRAY"/Tables/"
mkdir -p $LDIR

# delete old log files
rm -f $LDIR/$TFIL-$ARRAY.log
# delete old table file (mscw_energy would otherwise stop with an error message)
rm -f $LDIR/$TFIL-$ARRAY.root

# options for table filling
# set MOPT="$SETOFF -pe -filltables=1 -ze=20. -noise=250 -woff=0.0 -maxCoreError=250 -minImages=2 -limitEnergyReconstruction"
set MOPT="$SETOFF -pe -filltables=1 -ze=20. -noise=250 -woff=0.0 -maxCoreError=250 -minImages=2 -limitEnergyReconstruction -write1DHistograms"

#########################################
# fill tables
cd $EVNDISPSYS/bin/
./mscw_energy $MOPT -arrayrecid=$RECID -tablefile "$ODIR/$TFIL-$ARRAY.root" -inputfile "$DDIR*.root" > $LDIR/$TFIL-$ARRAY.log
#########################################

# sleep
sleep 2

exit
