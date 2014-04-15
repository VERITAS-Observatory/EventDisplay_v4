#!/bin/bash
# script to run evndisp for CARE simulations on one of the cluster nodes (VBF)
# Gernot Maier

# parameters replaced by parent script using sed
RUNNUM=RUNNUMBER
ZA=ZENITHANGLE
WOBBLE=WOBBLEOFF
NOISE=NOISELEVEL
EPOCH=ARRAYEPOCH
ATM=ATMOSPHERE
SIMDIR=DATADIR
ODIR=OUTPUTDIR
PARTICLE=PARTICLETYPE
TELTOANA="1234"

# Output file name
ONAME="CARE_${PARTICLE}_${ZA}deg_${WOBBLE}wob_NOISE${NOISE}_${EPOCH}_ATM${ATM}"

# input files (observe that these might need some adjustments)
[[ $PARTICLE == "1" ]] && VBFNAME="gamma_${ZA}deg_750m_${WOBBLE}wob_${NOISE}mhz_up_ATM${ATM}_part0"
[[ $PARTICLE == "2" ]] && VBFNAME="electron_${ZA}deg_noise${NOISE}MHz___"
[[ $PARTICLE == "14" ]] && VBFNAME="proton_${ZA}deg_noise${NOISE}MHz___"

# detector configuration and cuts
DEAD="EVNDISP.validchannels.dat"
ACUT="EVNDISP.reconstruction.runparameter"
[[ $EPOCH == "V4" ]] && CFG="EVN_V4_Oct2012_oldArrayConfig_20130428_v420.txt"
[[ $EPOCH == "V5" ]] && CFG="EVN_V5_Oct2012_newArrayConfig_20121027_v420.txt"
[[ $EPOCH == "V6" ]] && CFG="EVN_V6_Upgrade_20121127_v420_CARE.txt"

# temporary data directory
DDIR="$TMPDIR/evn_${ZA}_${NOISE}_${WOBBLE}"
mkdir -p $DDIR
echo $DDIR

# unzip vbf file to local scratch directory
VBF_FILE="$VBFNAME.cvbf"
echo "Now processing $VBF_FILE"

if [[ ! -e "$DDIR/$VBF_FILE" ]]; then
    if [[ -e "$SIMDIR/$VBF_FILE.gz" ]] then
    echo "Copying $SIMDIR/$VBF_FILE.gz to $DDIR"
    cp -f "$SIMDIR/$VBF_FILE.gz" $DDIR/
    echo " (cvbf file copied)"
    gunzip -f -v "$DDIR/$VBF_FILE.gz"
elif( -e "$SIMDIR/$VBF_FILE.bz2" ) then
    echo "Copying $SIMDIR/$VBF_FILE.bz2 to $DDIR"
    cp -f "$SIMDIR/$VBF_FILE.bz2" $DDIR/
    echo " (cvbf file copied)"
    bunzip2 -f -v "$DDIR/$VBF_FILE.bz2"
fi

# check that the uncompressed vbf file exists
if [[! -e "$DDIR/$VBF_FILE" ]]; then
    echo "No source file found: $DDIR/$VBF_FILE"
    echo "$SIMDIR/$VBF_FILE*"
    exit 1
fi
VBF_FILE="$DDIR/$VBF_FILE"

###############################################
# calculate pedestals
echo "Calculating pedestals for run $RUNNUM"
rm -f $ODIR/$RUNNUM.ped.log
$EVNDISPSYS/bin/evndisp -runmode=1 -sourcetype=2 -sourcefile $VBF_FILE -teltoana=$TELTOANA -runnumber=$RUNNUM -calibrationsumfirst=0 -calibrationsumwindow=20 -donotusedbinfo -calibrationdirectory $ODIR &> $ODIR/$RUNNUM.ped.log

###############################################
# calculate tzeros
echo "Calculating tzeros for run $RUNNUM"
rm -f $ODIR/$RUNNUM.tzero.log
$EVNDISPSYS/bin/evndisp -runmode=7 -sourcetype=2 -sourcefile $VBF_FILE -teltoana=$TELTOANA -runnumber=$RUNNUM -calibrationsumfirst=0 -calibrationsumwindow=20 -donotusedbinfo -calibrationnevents==50000 -calibrationdirectory $ODIR &> $ODIR/$RUNNUM.tzero.log

### eventdisplay run options
OPT="-reconstructionparameter $ACUT"

# pedestal and calibration options
PEDOPT="-pedestalnoiselevel=$NOISE -calibrationdirectory $ODIR"

# Monte-Carlo options
MCOPT="-shorttree -sourcetype=2 -camera=$CFG"

###############################################
# run eventdisplay 
$EVNDISPSYS/bin/evndisp -runnumber=$RUNNUM -sourcefile $VBF_FILE -deadchannelfile $DEAD -outputfile $DDIR/$ONAME.root -teltoana=$TELTOANA $MCOPT $PEDOPT $OPT &> $ODIR/$ONAME.log

# remove temporary files
cp -f -v $DDIR/$ONAME.root $ODIR/$ONAME.root
rm -f -v $DDIR/$ONAME.root
rm -f -v $VBF_FILE

echo "EVNDISP output root file written to $ODIR/$RUNNUM.root"
echo "EVNDISP log file written to $ODIR/$RUNNUM.dat"

exit
