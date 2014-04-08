#$ -S /bin/tcsh
#
# script to run evndisp for grisu simulations on one of the cluster nodes (VBF)
#
# Gernot Maier
#
##############################################################################################
# variables set by parent script
##############################################################################################
set ZEW=123456789
set WOB=987654321
set WOG=WOGWOG
set NOISE=NOISENOISE
set ARRAY=XXXXXX
set FDIR=DATADIR
set YDIR=OUTDIR
set PART=IDIDID
set ATMO=AAAAAAAA
set SFIXRUN=RUNRUNRUN
set SFILN=NFINFIL
set RECID=RECONSTRUCTIONID
##############################################################################################
# input files
# (observe that these might need some adjustments)
##############################################################################################

source $EVNDISPSYS/setObservatory.tcsh VERITAS

###################### V4 #################################################
if( $ARRAY == "V4" ) then
   if( $PART == "1" ) then
#      set IFIL=Nov10_oa_ATM"$ATMO"_"$ZEW"deg_"$WOG"
#       set IFIL=Apr12_oa_ATM"$ATMO"_"$ZEW"deg_"$WOG"
      if( $ATMO == "21" ) then
         set IFIL=Oct2012_oa_ATM"$ATMO"_"$ZEW"deg_"$WOG"
      endif
      if( $ATMO == "22" ) then
         set IFIL=Nov10_oa__ATM"$ATMO"_"$ZEW"deg_"$WOG"
      endif
      set RUN=( "wobb" )
      set SRUN=47460
      set NRUN=1
   endif
   if( $PART == "14" ) then
      set IFIL=proton_"$ZEW"deg_750m_wobble"$WOB"_2008_2009_
      set RUN=( 8"$ZEW"0 9"$ZEW"0 9"$ZEW"5 )
      set SRUN=47460
      set NRUN=3
   endif
   set CFG="EVN_V4_Autumn2007_20130110.txt"
   set CFG="EVN_V4_Oct2012_oldArrayConfig_20130428_v420.txt"
# noise file
   set NOISEFILE="$OBS_EVNDISP_ANA_DIR/NOISE/NOISE$NOISE.grisu"
   echo "NOISE FILE " $NOISEFILE
endif
###################### V5 #################################################
if( $ARRAY == "V5" ) then
   if( $PART == "1" ) then
      set IFIL=Nov10_na_ATM"$ATMO"_"$ZEW"deg_"$WOG"
      set IFIL=Apr12_na_ATM"$ATMO"_"$ZEW"deg_"$WOG"
      set IFIL=gamma_V5_Oct2012_newArrayConfig_20121027_v420_ATM"$ATMO"_"$ZEW"deg_"$WOG"
      set RUN=( "wobb" )
      set SRUN=47570
      set NRUN=1
   endif
   if( $PART == "14" ) then
      set IFIL=proton_"$ZEW"deg_w"$WOB"_
      set IFIL="PrePostUpgradeCompare_20130423.NC$SFILN.vbf.bz2"
      set RUN=( $SFILN )
      set NRUN=1
      set SRUN=$SFIXRUN
   endif
   if( $PART == "402" ) then
      set IFIL=helium_"$ZEW"deg_w"$WOB"_
      set RUN=( 800 )
      set NRUN=1
      set SRUN=47570
   endif
#   set CFG="veritasBC4N_090916_Autumn2009-4.1.5_EVNDISP.cfg"
   set CFG="EVN_V5_Oct2012_newArrayConfig_20121027_v420.txt"
# noise file
   set NOISEFILE="$OBS_EVNDISP_ANA_DIR/NOISE/NOISE$NOISE.grisu"
   echo "NOISE FILE " $NOISEFILE
endif
###################### V6 #################################################
if( $ARRAY == "V6" ) then
   if( $PART == "1" ) then
      set IFIL=UPG_V0_"$ZEW"deg_"$WOG"
      set IFIL=gamma_V6_Upgrade_20121127_v420_ATM"$ATMO"_"$ZEW"deg_"$WOG"
      if( $ATMO == "21-redHV" ) then
         set IFIL=gamma_V6_Upgrade_ReducedHV_20121211_v420_ATM21_"$ZEW"deg_"$WOG"
      endif
      if( $ATMO == "21-UV" ) then
#         set IFIL=gamma_V6_Upgrade_UVfilters_20121211_v420_ATM21_"$ZEW"deg_"$WOG"
         set IFIL=gamma_grisu_apr12_ATM21_V6_Upgrade_UVfilters_20131025_"$ZEW"deg_"$WOG"
      endif
      if( $ATMO == "21-SNR" ) then
         set IFIL=gamma_V6_201304_SN2013ak_v420_ATM21_"$ZEW"deg_"$WOG"
      endif
      set RUN=( "wobb" )
      set SRUN=65000
      set NRUN=1
   endif
   if( $PART == "14" ) then
      set IFIL=proton_"$ZEW"deg_w"$WOB"_
      set IFIL="PrePostUpgradeCompare_20130423.NC"
      set RUN=( $SFILN )
      set NRUN=1
      set SRUN=$SFIXRUN
   endif
   if( $PART == "402" ) then
      set IFIL=helium_"$ZEW"deg_w"$WOB"_
      set RUN=( 800 )
      set NRUN=1
      set SRUN=47570
   endif
   set CFG="EVN_V6_Upgrade_20121127_v420.txt"
# noise file (V6!)
   set NOISEFILE="$OBS_EVNDISP_ANA_DIR/NOISE/NOISE$NOISE"_20120827_v420.grisu
   echo "NOISE FILE " $NOISEFILE
endif

##############################################################################################
# directory with executable
cd $EVNDISPSYS/scripts/VTS/
##############################################################################################
# detector configuration and cuts
# telescopes
set TTA="1234"
#set TTA="234"
# set ACUT="EVNDISP.reconstruction.CORR.runparameter"
set ACUT="EVNDISP.reconstruction.runparameter"
# dead channel definitions
set DEAD="EVNDISP.validchannels.dat"
# default pedestal level
set PEDLEV="16."
##############################################################################################

#####################################################
# temporary data directory
set DDIR=$TMPDIR"/evn_"$ZEW"_"$NOISE"_"$WOB
echo $DDIR
mkdir -p $DDIR

##############################################################################################
# loop over all run numbers
##############################################################################################
set i = 1
while ($i <= $NRUN)
  set RRR=$RUN[$i]
  echo $RRR
##############################################################################################
# unzip vbf file to local scratch directory
##############################################################################################
  set VFIL=$IFIL"$RRR".vbf
  if (! -e $DDIR/$VFIL ) then
    if ( -e $FDIR/$IFIL"$RRR".vbf.gz ) then
       echo "copying $FDIR/$IFIL"$RRR".vbf.gz to $DDIR"
       cp -f $FDIR/$IFIL"$RRR".vbf.gz $DDIR/
       echo " (vbf file copied, was gzipped)"
       gunzip -f -v $DDIR/$IFIL"$RRR".vbf.gz
    else if( -e $FDIR/$IFIL"$RRR".vbf.bz2 ) then
       echo "copying $FDIR/$IFIL"$RRR".vbf.bz2 to $DDIR"
       cp -f $FDIR/$IFIL"$RRR".vbf.bz2 $DDIR/
       echo " (vbf file copied, was bzipped)"
       bunzip2 -f -v $DDIR/$IFIL"$RRR".vbf.bz2
    else if( -e $FDIR/$IFIL"$RRR".vbf ) then
       echo "copying $VFIL to $DDIR"
       cp -f $FDIR/$VFIL $DDIR/
    endif
  endif
  set XFIL=$DDIR/$IFIL"$RRR".vbf
  if (! -e $XFIL ) then
     echo "no source file found: $XFIL"
     echo "$FDIR/$IFIL"$RRR".vbf*"
     exit
  endif
  echo "SOURCE FILE " $XFIL

##############################################################################################
# define run numbers
##############################################################################################
  if( $RRR == "wobb" ) then
     set RRR="100"$ZEW
  endif

##############################################################################################
# output directory
##############################################################################################
set ODIR=$YDIR/analysis_ATM"$ATMO"_"$TTA"_NOISE"$NOISE"/
mkdir -p $ODIR

##############################################################################################
# eventdisplay run options
##############################################################################################

##### pedestal options #####
set PEDOPT="-pedestalfile $NOISEFILE -pedestalseed=$RRR -pedestalDefaultPedestal=$PEDLEV -pedestalnoiselevel=$NOISE"

##### MC options #####
# restrict average tzero calculation to inner part of the camera
# set MCOPT="-sourcetype=2 -camera=$CFG -averagetzerofiducialradius=0.60"
# default options
set MCOPT="-noshorttree -sourcetype=2 -camera=$CFG"

echo "RUNNUMBER $SRUN"
echo "EVNDISP outputfile root file written to $ODIR/$RRR.root"
echo "EVNDISP log file written to $ODIR/$RRR.log"

##############################################################################################
# calculating tzeros
##############################################################################################
##echo "CALCULATING AVERAGE TZEROS FOR RUN $SRUN"
##rm -f $ODIR/$SRUN.tzero.log
##$EVNDISPSYS/bin/evndisp -sourcetype=2 -sourcefile $XFIL -teltoana=$TTA -runmode=7 -runnumber=$SRUN -deadchannelfile $DEAD -arraycuts $ACUT -calibrationsumwindow=20 -calibrationsumfirst=0 -donotusedbinfo -calibrationnevents==100000 $PEDOPT -calibrationdirectory $ODIR >& $ODIR/$SRUN.tzero.log

set FROGS="-frogs $VERITAS_USER_DATA_DIR/analysis/EVDv400/"$ARRAY"_FLWO/mscw_ATM"$ATMO"/gamma_"$ZEW"deg_750m_w"$WOB"_ID"$RECID"_ana1234_NOISE"$NOISE"_1.root -frogid 0"

##############################################################################################
# run eventdisplay 
##############################################################################################
$EVNDISPSYS/bin/evndisp -runnumber=$SRUN -nevents=3000000 -writenomctree -sourcefile $XFIL -deadchannelfile $DEAD -arraycuts $ACUT $FROGS -outputfile $DDIR/$RRR.root -teltoana=$TTA $MCOPT $PEDOPT -calibrationdirectory $ODIR >& $ODIR/$RRR.log
##############################################################################################

  sleep 10s

  mv -f -v $DDIR/$RRR.root $ODIR/$RRR.root

# remove temporary vbf file
  rm -f -v $XFIL

  @ i = $i + 1
end

exit