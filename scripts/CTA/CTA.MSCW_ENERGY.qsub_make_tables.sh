#!/bin/tcsh
#
# make tables for CTA
#
#
# Author: Gernot Maier
#

set TFIL=TABLEFILE
set RECID=RECONSTRUCTIONID
set WOFFMIN=WOMIIIIIN
set WOFFMAX=WOMAXXXXX
set WOFFMEA=WOMEEEEAN
set ARRAY=ARRRRRRR

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

# output data files are written to this directory
set ODIR=$CTA_USER_DATA_DIR"/analysis/"$ARRAY"/Tables/"
mkdir -p $ODIR

# output log files are written to this directory
set LDIR=$CTA_USER_LOG_DIR"/analysis/"$ARRAY"/Tables/"
mkdir -p $LDIR

# delete old log files
rm -f $LDIR/$TFIL-W$WOFFMEA.log

# location of EVNDISP output files (this is the input for the table filling)
# TTT set DDIR="$CTA_USER_DATA_DIR/analysis/$ARRAY/gamma_onSource/*.root"
set DDIR="$CTA_DATA_DIR/analysis/$ARRAY/gamma_onSource/3*.root"

# options for table filling
set MOPT="-pe -filltables=1 -ze=20. -noise=250 -woff=$WOFFMEA -mindistancetocameracenter=$WOFFMIN -maxdistancetocameracenter=$WOFFMAX -maxCoreError=250"

#########################################
# fill tables
cd $EVNDISPSYS/bin/
./mscw_energy $MOPT -arrayrecid=$RECID -tablefile "$ODIR/$TFIL-W$WOFFMEA.root" -inputfile "$DDIR" > $LDIR/$TFIL-W$WOFFMEA.log
#########################################

# sleep
sleep 2

exit
