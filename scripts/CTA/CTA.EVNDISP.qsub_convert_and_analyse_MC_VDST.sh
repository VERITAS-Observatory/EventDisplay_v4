#!/bin/tcsh
#
# script to convert sim_tel output files to EVNDISP DST file and then run eventdisplay
#
# Revision $Id: qsub_CTA_evndisp.sh,v 1.1.2.1.2.1.2.1 2011/04/06 11:59:26 gmaier Exp $
#
# Author: Gernot Maier
#

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

set IFIL=SIMTELFILE
set PART=PAAART
set SUBA=ARRAY
set KEEP=KEEEEEEP
set ACUT=ARC
set MET=MEEET

set OFIL=`basename $IFIL .gz`

# output data files are written to this directory
set ODIR=$CTA_USER_DATA_DIR"/analysis/"$SUBA"/"$PART"/"
mkdir -p $ODIR

# output log files are written to this directory
set LDIR=$CTA_USER_LOG_DIR"/analysis/"$SUBA"/"$PART"/"
mkdir -p $LDIR

####################################################################
# execute converter

# remove existing log files
rm -f $LDIR/$OFIL.convert.log
rm -f $LDIR/$OFIL.evndisp.log

# cp simtelarray.gz file to TMPDIR
cp -v -f $IFIL $TMPDIR"/"

$EVNDISPSYS/bin/CTA.convert_hessio_to_VDST -a $CTA_EVNDISP_ANA_DIR/DetectorGeometry/CTA.prod1.$SUBA.lis -o $TMPDIR/$OFIL.root $TMPDIR/$OFIL.gz >& $LDIR/$OFIL.convert.log

####################################################################
# execute eventdisplay

set OPT="-useFixedThresholds -shorttree -l2setspecialchannels nofile"
# GEO (standard reconstruction)
if( $MET == "GEO" ) then
   set OPT="$OPT"
endif
# LL
if( $MET == "LL" ) then
   set OPT="$OPT -loglminloss=0.00"
endif

$EVNDISPSYS/bin/evndisp -sourcefile $TMPDIR/$OFIL.root -reconstructionparameter $ACUT $OPT -outputdirectory $TMPDIR >& $LDIR/$OFIL.evndisp.log

####################################################################
# move evndisp files to data directory
if ( $KEEP == "1" ) then
   mkdir -p $ODIR/VDST
   cp -f $TMPDIR/$OFIL.root $ODIR/VDST/
endif
rm -f $TMPDIR/$OFIL.root

cp -v -f $TMPDIR/*.root $ODIR

exit

