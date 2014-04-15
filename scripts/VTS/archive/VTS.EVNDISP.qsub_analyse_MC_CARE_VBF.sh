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
   set IFIL=gamma_"$ZEW"deg_750m_"$WOB"wob_"$NOISE"mhz_up_ATM"$ATMO"_part0
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
# dead channel definitions
set DEAD="EVNDISP.validchannels.dat"
# camera geometry
set CFG="EVN_V6_Upgrade_20121127_v420.txt"
if( $ARRAY == "V4" ) then
    set CFG="EVN_V4_Oct2012_oldArrayConfig_20130428_v420.txt"
endif
if( $ARRAY == "V5" ) then
    set CFG="EVN_V5_Oct2012_newArrayConfig_20121027_v420.txt"
endif
if( $ARRAY == "V6" ) then
    set CFG="EVN_V6_Upgrade_20121127_v420_CARE.txt"
endif
##############################################################################################

#####################################################
# temporary data directory
set DDIR=$TMPDIR"/evn_"$ZEW"_"$RECFILE"_"$NOISE"_"$WOB
echo $DDIR
mkdir -p $DDIR

##############################################################################################
# unzip vbf file to local scratch directory
##############################################################################################
set VFIL=$IFIL.cvbf
echo "SOURCEFILE $VFIL"
if (! -e $DDIR/$VFIL ) then
 if ( -e $FDIR/$VFIL.gz ) then
    echo "copying $FDIR/$VFIL.gz to $DDIR"
    cp -f $FDIR/$VFIL.gz $DDIR/
    echo " (cvbf file copied)"
    gunzip -f -v $DDIR/$VFIL.gz
 else if( -e $FDIR/$VFIL.bz2 ) then
    echo "copying $FDIR/$VFIL.bz2 to $DDIR"
    cp -f $FDIR/$VFIL.bz2 $DDIR/
    echo " (cvbf file copied)"
    bunzip2 -f -v $DDIR/$VFIL.bz2
 endif
endif
set XFIL=$DDIR/$VFIL
if (! -e $XFIL ) then
  echo "no source file found: $XFIL"
  echo "$FDIR/$VFIL*"
  exit
endif

##############################################################################################
# output directory
##############################################################################################
set ODIR=$YDIR/analysisCARE_d20140403_ATM"$ATMO"_"$TTA"_NOISE"$NOISE"/
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

#### Low gain calibration ####
if (! -e $ODIR/calibrationlist.LowGain.dat ) then
    mkdir -p $ODIR/Calibration
    cp -f $VERITAS_EVNDISP_AUX_DIR/Calibration/calibrationlist.LowGain.dat $ODIR/Calibration/
endif

##############################################################################################
# calculate pedestals
##############################################################################################

echo "CALCULATING PEDESTALS FOR RUN $RUN"
rm -f $ODIR/$RUN.ped.log
$EVNDISPSYS/bin/evndisp -sourcetype=2 -sourcefile $XFIL -teltoana=$TTA -runmode=1 -runnumber=$RUN  -calibrationsumwindow=20 -calibrationsumfirst=0 -donotusedbinfo -calibrationdirectory $ODIR >& $ODIR/$RUN.ped.log

echo "CALCULATING AVERAGE TZEROS FOR RUN $RUN"
rm -f $ODIR/$RUN.tzero.log
$EVNDISPSYS/bin/evndisp -sourcetype=2 -sourcefile $XFIL -teltoana=$TTA -runmode=7 -runnumber=$RUN  -calibrationsumwindow=20 -calibrationsumfirst=0 -donotusedbinfo -calibrationnevents==50000 -calibrationdirectory $ODIR >& $ODIR/$RUN.tzero.log

##############################################################################################
# eventdisplay run options
##############################################################################################

##### reconstruction parameter file #####
set OPT="-reconstructionparameter $RECFILE"

##### pedestal and calibration options #####
set PEDOPT="-pedestalnoiselevel=$NOISE -calibrationdirectory $ODIR"

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
