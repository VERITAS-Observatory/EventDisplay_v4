#$ -S /bin/tcsh
#
# script to run evndisp on one of the cluster nodes (VBF)
#
# Gernot Maier
#
##############################################################################################
# variables set by parent script
##############################################################################################
set RUN=UUUAAA
set ZEW=123456789
set WOB=987654321
set WOG=WOGWOG
set NOISE=NOISENOISE
set ARRAY=XXXXXX
set FDIR=DATADIR
set YDIR=OUTDIR
set RECFILE=ACUT
set PART=IDIDID
set ATMO=AAAAAAAA
##############################################################################################
# input files
# (observe that these might need some adjustments)
##############################################################################################

source $EVNDISPSYS/setObservatory.tcsh VERITAS

if( $PART == "1" ) then
   set IFIL=gamma_"$ZEW"deg_"$WOG"wobble_noise"$NOISE"MHz___
endif
if( $PART == "2" ) then
   set IFIL=electron_"$ZEW"deg_noise"$NOISE"MHz___
endif
if( $PART == "14" ) then
   set IFIL=proton_"$ZEW"deg_noise"$NOISE"MHz___
endif

##############################################################################################
# directory with executable
cd $EVNDISPSYS/scripts/VTS/
##############################################################################################
# detector configuration and cuts
# telescopes
set TTA="1234"
#set TTA="234"
# dead channel definitions
set DEAD="EVNDISP.validchannels.dat"
# camera geometry
set CFG="veritasBC4N_090916_Autumn2009-4.1.5_EVNDISP.cfg"
##############################################################################################

#####################################################
# temporary data directory
set DDIR=$TMPDIR"/evn_"$ZEW"_"$RECFILE"_"$NOISE"_"$WOB
echo $DDIR
mkdir -p $DDIR

##############################################################################################
# unzip vbf file to local scratch directory
##############################################################################################
set VFIL=$IFIL"$RUN".vbf
echo "SOURCEFILE $FDIR/$IFIL"$RUN".vbf.gz"
if (! -e $DDIR/$VFIL ) then
 if ( -e $FDIR/$IFIL"$RUN".vbf.gz ) then
    echo "copying $FDIR/$IFIL"$RUN".vbf.gz to $DDIR"
    cp -f $FDIR/$IFIL"$RUN".vbf.gz $DDIR/
    echo " (vbf file copied)"
    gunzip -f -v $DDIR/$IFIL"$RUN".vbf.gz
 else if( -e $FDIR/$IFIL"$RUN".vbf.bz2 ) then
    echo "copying $FDIR/$IFIL"$RUN".vbf.bz2 to $DDIR"
    cp -f $FDIR/$IFIL"$RUN".vbf.bz2 $DDIR/
    echo " (vbf file copied)"
    bunzip2 -f -v $DDIR/$IFIL"$RUN".vbf.bz2
 endif
endif
set XFIL=$DDIR/$IFIL"$RUN".vbf
if (! -e $XFIL ) then
  echo "no source file found: $XFIL"
  echo "$FDIR/$IFIL"$RUN".vbf*"
  exit
endif

##############################################################################################
# output directory
##############################################################################################
set ODIR=$YDIR/analysis_d20121026_ATM"$ATMO"_"$TTA"_"$RECFILE"_NOISE"$NOISE"/
mkdir -p $ODIR

##############################################################################################
#fix run numbers
##############################################################################################
if( $ARRAY == "V4" ) then
   set RUN="94$RUN"
endif
if( $ARRAY == "V5" ) then
   set RUN="95$RUN"
endif
if( $ARRAY == "V6" ) then
   set RUN="96$RUN"
endif

##############################################################################################
# calculate pedestals
##############################################################################################

echo "CALCULATING PEDESTALS FOR RUN $RUN"
rm -f $ODIR/$RUN.ped.log
$EVNDISPSYS/bin/evndisp -sourcetype=2 -sourcefile $XFIL -teltoana=$TTA -runmode=1 -runnumber=$RUN  -calibrationsumwindow=20 -calibrationsumfirst=0 -donotusedbinfo -nevents=180 >& $ODIR/$RUN.ped.log

echo "CALCULATING AVERAGE TZEROS FOR RUN $RUN"
rm -f $ODIR/$RUN.tzero.log
$EVNDISPSYS/bin/evndisp -sourcetype=2 -sourcefile $XFIL -teltoana=$TTA -runmode=7 -runnumber=$RUN  -calibrationsumwindow=20 -calibrationsumfirst=0 -donotusedbinfo -nevents=500000 >& $ODIR/$RUN.ped.log

set CALIBDATA=$OBS_EVNDISP_ANA_DIR/Calibration/calibrationlist.dat
if (! -e $CALIBDATA ) then
  touch $CALIBDATA 
endif
echo "*V4 $RUN -1 $RUN -1 -1 -1 -1 -1 -1 -1 -1" >> $CALIBDATA

##############################################################################################
# eventdisplay run options
##############################################################################################

##### reconstruction parameter file #####
set OPT="-reconstructionparameter $RECFILE"

##### pedestal options #####
set PEDOPT="-calibrationfile calibrationlist.dat -pedestalnoiselevel=$NOISE"

##### MC options #####
set MCOPT="-shorttree -sourcetype=2 -camera=$CFG"

echo "RUNNUMBER $RUN"
echo "EVNDISP outputfile root file written to $ODIR/$RUN.root"
echo "EVNDISP log file written to $ODIR/$RUN.dat"

##############################################################################################
# run eventdisplay 
##############################################################################################
$EVNDISPSYS/bin/evndisp -runnumber=$RUN -sourcefile $XFIL -deadchannelfile $DEAD -outputfile $ODIR/$RUN.root -teltoana=$TTA $MCOPT $PEDOPT $OPT  >& $ODIR/$RUN.log

##############################################################################################

# remove temporary vbf file
rm -f -v $XFIL

exit
