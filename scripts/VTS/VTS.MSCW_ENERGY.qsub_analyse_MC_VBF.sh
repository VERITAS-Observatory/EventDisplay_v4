#!/bin/tcsh
#
# script to analyse MC files with lookup tables
#
# Revision $Id: qsub_analyse_MC.sh,v 1.1.2.2.6.2.2.2.6.3.2.4.2.12.2.5.6.2 2011/04/19 07:39:44 gmaier Exp $
#
# Author: Gernot Maier
#
###############################################
# values filled by parent script
###############################################
set TFIL=TABLEFILE
set ANAC=TELESCOPES
set RECID=RECONSTRUCTIONID
set ZE=IZENITH
set NOISE=NNNNOISE
set WOFF=WWWOBB
set PART=PAAAAART
set RUNN=RUUUUNNN
set ATMOS=ATMOOOS
set FTRE=FUUUUUL
set ARRAY=ARRRRAY

cd $EVNDISPSYS/
source ./setObservatory.tcsh VTS

###############################################
# hardwired values
###############################################
# date of analysis
set DAT="d20120410"
# output files are written to this directory
set ODIR="$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/mscw_ATM"$ATMOS"_"$DAT"
# directory with MC eventdisplay files
set SDIR="analysis_"$DAT"_ATM"$ATMOS"_"$ANAC"_NOISE"$NOISE
# full path to MC eventdisplay files 
set XDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/"$PART"_"$ZE"deg_750m/wobble_$WOFF/$SDIR/$RUNN*.root"
# mscw_energy command line options
set MOPT="-noNoTrigger -nomctree -shorttree -writeReconstructedEventsOnly=1 -arrayrecid=$RECID -tablefile $TFIL.root"
if( $FTRE == "TRUE" ) then
  set ODIR="$ODIR"G
  set MOPT="-noNoTrigger -writeReconstructedEventsOnly=1 -arrayrecid=$RECID -tablefile $TFIL.root"
endif

###############################################
# temporary directories
###############################################
set DDIR=$TMPDIR"/mscwMCSW"$ZE"deg"$WOFF"degNOISE"$NOISE"ID"$RECID
mkdir -p $DDIR

###############################################
# output file name
###############################################
set OFIL="$PART"_"$ZE"deg_750m_w"$WOFF"_ID"$RECID"_ana"$ANAC"_NOISE$NOISE"_"$RUNN
mkdir -p $ODIR

###############################################
# run MSCW
###############################################
cd $EVNDISPSYS/bin/
./mscw_energy $MOPT -inputfile "$XDIR" -outputfile $DDIR/$OFIL.root -noise=$NOISE > $ODIR/$OFIL.log

###############################################
# cp results file back to data directory and clean up
###############################################
cp -f -v $DDIR/$OFIL.root $ODIR/
rm -r -f $DDIR

exit
