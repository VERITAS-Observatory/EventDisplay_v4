#$ -S /bin/tcsh
#
# script to analyse MC files with lookup tables
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
set ARRAY=ARRRRAY
set SIM=SIMS

cd $EVNDISPSYS/
source ./setObservatory.tcsh VTS

#################################################
# hardwired values (depend on simulation package)
#################################################
# GRISUDET
if( $SIM == "GRISU" ) then
# date of analysis
    set DAT="d20131031"
# output files are written to this directory
    set ODIR="$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/mscw_ATM"$ATMOS"_"$DAT"
#    set ODIR=/lustre/fs9/group/cta/users/maierg/VERITAS/analysis/EVDv400/"$ARRAY"_FLWO/mscw_ATM"$ATMOS"_"$DAT"
# directory with MC eventdisplay files
    set SDIR="analysis_"$DAT"_ATM"$ATMOS"_"$ANAC"_NOISE"$NOISE
# full path to MC eventdisplay files 
    set XDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/"$PART"_"$ZE"deg_750m/wobble_$WOFF/$SDIR/$RUNN*[0-9].root"
else if( $SIM == "CARE" ) then
# date of analysis
    set DAT="d20140127"
# output files are written to this directory
# set ODIR="$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/mscw_ATM"$ATMOS"_"$DAT"
    set ODIR=/lustre/fs9/group/cta/users/maierg/VERITAS/analysis/EVDv400/"$ARRAY"_FLWO/mscw_CARE_ATM"$ATMOS"_"$DAT"
# directory with MC eventdisplay files
    set SDIR="analysisCARE_"$DAT"_ATM"$ATMOS"_"$ANAC"_NOISE"$NOISE
# full path to MC eventdisplay files 
    set XDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/care_Jan1427/"$PART"_"$ZE"deg_750m/wobble_$WOFF/$SDIR/$RUNN*[0-9].root"
endif 
#############
# mscw_energy command line options
# long output file (with all telescope-wise image variables)
# set MOPT="-noNoTrigger -nomctree -writeReconstructedEventsOnly=1 -arrayrecid=$RECID -tablefile $TFIL.root"
# short output file
set MOPT="-noNoTrigger -nomctree -shorttree -writeReconstructedEventsOnly=1 -arrayrecid=$RECID -tablefile $TFIL.root"

###############################################
# temporary directory
###############################################
if ($?TMPDIR) then 
    set DDIR=$TMPDIR"/mscwMCSW"$ZE"deg"$WOFF"degNOISE"$NOISE"ID"$RECID
else
    set DDIR=/tmp"/mscwMCSW"$ZE"deg"$WOFF"degNOISE"$NOISE"ID"$RECID
endif
mkdir -p $DDIR
echo "temporary directory $DDIR"

###############################################
# output file name
###############################################
set OFIL="$PART"_"$ZE"deg_750m_w"$WOFF"_ID"$RECID"_ana"$ANAC"_NOISE$NOISE"_"$RUNN
mkdir -p $ODIR

###############################################
# run MSCW
###############################################
cd $EVNDISPSYS/bin/
rm -f $ODIR/$OFIL.log
./mscw_energy $MOPT -inputfile "$XDIR" -outputfile $DDIR/$OFIL.root -noise=$NOISE > $ODIR/$OFIL.log

###############################################
# cp results file back to data directory and clean up
###############################################
cp -f -v $DDIR/$OFIL.root $ODIR/
rm -r -f $DDIR

exit
