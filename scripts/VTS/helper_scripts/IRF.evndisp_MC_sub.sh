#!/bin/bash
# script to run evndisp for simulations on one of the cluster nodes (VBF)

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
RUNNUM=RUNNUMBER
SIMDIR=DATADIR
ZA=ZENITHANGLE
WOB=DECIMALWOBBLE
WOG=INTEGERWOBBLE
NOISE=NOISELEVEL
EPOCH=ARRAYEPOCH
ATM=ATMOSPHERE
ACUTS=RECONSTRUCTIONRUNPARAMETERFILE
PARTICLE=PARTICLETYPE
SIMTYPE=SIMULATIONTYPE
ODIR=OUTPUTDIR
NEVENTS=NENEVENT
TELTOANA="1234"
VBFNAME=VBFFFILE
NOISEFILE=NOISEFFILE

# Output file name
PEDNEVENTS="200000"
TZERONEVENTS="100000"

if [[ $NEVENTS -gt 0 ]]; then
    ITER=$((SGE_TASK_ID - 1))
    FIRSTEVENT=$(($ITER * $NEVENTS))
    # Output file name
    RUNNUM=$((RUNNUM + $ITER))
    # Output file name
    #ONAME="${RUNNUM}_$ITER"
    echo -e "ITER $ITER NEVENTS $NEVENTS FIRSTEVENT $FIRSTEVENT"
fi

# Output file name
ONAME="$RUNNUM"
echo "Runnumber $RUNNUM"
#################################
# detector configuration and cuts

echo "Using run parameter file $ACUTS"
# no disp, long integration window
DEAD="EVNDISP.validchannels.dat"
PEDLEV="16."
# LOWPEDLEV="8."
LOWPEDLEV="16."

# Amplitude correction factor options
AMPCORR="-traceamplitudecorrection MSCW.sizecal.runparameter -pedestalDefaultPedestal=$PEDLEV"
# CARE simulations: add Gaussian noise of 3.6 mV/ (7.84 mV/dc)  / 2
# Current (2018) CARE simulations:
#    no electronic noise included - therefore add
#    Gaussian noise with the given width
#    Derived for GrIsu many years ago - source not entirely clear
#    add Gaussian noise of 3.6 mV/ (7.84 mV/dc)  / 2
if [[ ${SIMTYPE:0:4} == "CARE" ]]; then
    AMPCORR="$AMPCORR -injectGaussianNoise=0.229592"
fi

# detector configuration
[[ ${EPOCH:0:2} == "V4" ]] && CFG="EVN_V4_Oct2012_oldArrayConfig_20130428_v420.txt"
[[ ${EPOCH:0:2} == "V5" ]] && CFG="EVN_V5_Oct2012_newArrayConfig_20121027_v420.txt"
[[ ${EPOCH:0:2} == "V6" ]] && CFG="EVN_V6_Upgrade_20121127_v420.txt"
    

# temporary directory
if [[ -n "$TMPDIR" ]]; then 
    DDIR="$TMPDIR/evn_${ZA}_${NOISE}_${WOG}"
else
    DDIR="/tmp/evn_${ZA}_${NOISE}_${WOG}"
fi
mkdir -p $DDIR
echo "Temporary directory: $DDIR"

# loop over simulation files
if [[ ${SIMTYPE:0:5} == "GRISU" ]]; then
    VBF_FILE=$VBFNAME"wobb.vbf"
elif [[ ${SIMTYPE:0:10} == "CARE_RedHV" ]]; then
    VBF_FILE="$VBFNAME.cvbf.zst"
elif [[ ${SIMTYPE:0:4} == "CARE" ]]; then
    VBF_FILE="$VBFNAME.cvbf"
fi
echo 
echo "Now processing $VBF_FILE"

# unzip vbf file to local scratch directory
if [[ ! -f "$DDIR/$VBF_FILE" ]]; then
    if [[ -e "$SIMDIR/$VBF_FILE.gz" ]]; then
        echo "Copying $SIMDIR/${VBF_FILE}.gz to $DDIR"
        cp -f "$SIMDIR/$VBF_FILE.gz" $DDIR/
        echo " (vbf file copied, was gzipped)"
        gunzip -f -q "$DDIR/$VBF_FILE.gz"
    elif [[ -e "$SIMDIR/$VBF_FILE.zst" ]]; then
        # check if zstd if installed
        if hash zstd 2>/dev/null; then
            echo "Unzipping $SIMDIR/${VBF_FILE}.zst to $DDIR"
            ls -l "$SIMDIR/$VBF_FILE.zst" 
            zstd -d -f "$SIMDIR/$VBF_FILE.zst" -o "$DDIR/$VBF_FILE"
        else
            echo "no zstd installed; exiting"
            exit
        fi
    elif [[ -e "$SIMDIR/$VBF_FILE.bz2" ]]; then
        echo "Copying $SIMDIR/$VBF_FILE.bz2 to $DDIR"
        cp -f "$SIMDIR/$VBF_FILE.bz2" $DDIR/
        echo " (vbf file copied, was bzipped)"
        bunzip2 -f -q "$DDIR/$VBF_FILE.bz2"
    elif [[ -e "$SIMDIR/$VBF_FILE" ]]; then
        echo "Copying $VBF_FILE to $DDIR"
        cp -f "$SIMDIR/$VBF_FILE" $DDIR/
    fi
fi

# check that the uncompressed vbf file exists
if [[ ! -f "$DDIR/$VBF_FILE" ]]; then
    echo "No source file found: $DDIR/$VBF_FILE"
    echo "$SIMDIR/$VBF_FILE*"
    exit 1
fi
VBF_FILE="$DDIR/$VBF_FILE"

# Low gain calibration
mkdir -p $ODIR/Calibration
if [[ ! -f $ODIR/Calibration/calibrationlist.LowGain.dat ]]; then 
    if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
        cp -f $VERITAS_EVNDISP_AUX_DIR/Calibration/calibrationlist.LowGain.dat $ODIR/Calibration/calibrationlist.LowGain.dat
    elif [ ${SIMTYPE:0:4} = "CARE" ]; then
        cp -f $VERITAS_EVNDISP_AUX_DIR/Calibration/calibrationlist.LowGainForCare.dat $ODIR/Calibration/calibrationlist.LowGainForCare.dat
    fi
fi

###############################################
# calculate pedestals
# (CARE only, GRISU used external noise file)
if [[ ${SIMTYPE:0:4} == "CARE" ]]; then
    echo "Calculating pedestals for run $RUNNUM"
    rm -f $ODIR/$RUNNUM.ped.log
    $EVNDISPSYS/bin/evndisp -runmode=1 -sourcetype=2 -epoch $EPOCH -sourcefile $VBF_FILE -runnumber=$RUNNUM -calibrationsumfirst=0 -calibrationsumwindow=20 -donotusedbinfo $AMPCORR -calibrationdirectory $ODIR &> $ODIR/$RUNNUM.ped.log
fi    

###############################################
# calculate tzeros
if [[ $USEFROGS != "1" ]]; then
    echo "Calculating average tzeros for run $RUNNUM"
    MCOPT="-runmode=7 -sourcetype=2 -epoch $EPOCH -camera=$CFG -sourcefile $VBF_FILE -runnumber=$RUNNUM -calibrationsumfirst=0 -calibrationsumwindow=20 -donotusedbinfo -calibrationnevents=100000 -calibrationdirectory $ODIR -reconstructionparameter $ACUTS -pedestalnoiselevel=$NOISE "
    rm -f $ODIR/$RUNNUM.tzero.log
    ### eventdisplay GRISU run options
    if [[ ${SIMTYPE:0:5} = "GRISU" ]]; then
        MCOPT="$MCOPT -pedestalfile $NOISEFILE -pedestalseed=$RUNNUM -pedestalDefaultPedestal=$PEDLEV -lowgaincalibrationfile NOFILE -lowgainpedestallevel=$PEDLEV"
    else
       MCOPT="$MCOPT -lowgainpedestallevel=$LOWPEDLEV -lowgaincalibrationfile calibrationlist.LowGainForCare.dat"
    fi
    echo "$EVNDISPSYS/bin/evndisp $MCOPT" &> $ODIR/$RUNNUM.tzero.log
    $EVNDISPSYS/bin/evndisp $MCOPT $AMPCORR &>> $ODIR/$RUNNUM.tzero.log
fi

###############################################
# run eventdisplay
###############################################
# model 3D
if [[ $USEMODEL3D == "1" ]]; then
    MODEL3D="-model3d -lnlfile $VERITAS_EVNDISP_AUX_DIR/Model3D/table_Model3D_Likelihood.root"
fi
# FROGS
if [[ $USEFROGS == "1" ]]; then
	 MSCWFILE="${ZA}deg_${WOB}wob_NOISE${NOISE}_${ITER}.mscw.root"
    echo -e "FROGS MSCW Dir:\n $MSCWDIR"
    echo -e "FROGS MSCW File:\n $MSCWFILE"
    echo "FROGS NEvents: $NEVENTS"
   # template list file
   if [[ "$EPOCH" =~ ^(V5|V6)$ ]]; then
      TEMPLATELIST="EVNDISP.frogs_template_file_list.$EPOCH.txt"
   else
      echo "Error (helper_scripts/IRF.evndisp_MC_sub.sh), no frogs template list defined for $EPOCH='$EPOCH', exiting..."
      exit 1
   fi
   echo "Using template list file '$TEMPLATELIST'..."
	echo "$MSCWDIR/$MSCWFILE $NEVENTS $FIRSTEVENT"
   FROGS="-frogs $MSCWDIR/$MSCWFILE -frogsid 0 -templatelistforfrogs "$TEMPLATELIST" -frogsparameterfile FROGS.runparameter"
fi
# run options
MCOPT=" -runnumber=$RUNNUM -sourcetype=2 -epoch $EPOCH -camera=$CFG -reconstructionparameter $ACUTS -sourcefile $VBF_FILE  -writenomctree -deadchannelfile $DEAD -outputfile $DDIR/$ONAME.root -donotusedbinfo -calibrationdirectory $ODIR"
# special options for GRISU
if [[ ${SIMTYPE:0:5} == "GRISU" ]]; then
    MCOPT="$MCOPT -simu_hilo_from_simfile -pedestalfile $NOISEFILE -pedestalseed=$RUNNUM -pedestalDefaultPedestal=$PEDLEV -lowgaincalibrationfile NOFILE -lowgainpedestallevel=$PEDLEV"
else
    MCOPT="$MCOPT -lowgainpedestallevel=$LOWPEDLEV -lowgaincalibrationfile calibrationlist.LowGainForCare.dat"
fi
# throughput correction after trace integration
# do not combine with corretion of FADC values (!)
# MCOPT="$MCOPT -throughputcorrection MSCW.sizecal.runparameter"
if [[ $NEVENTS -gt 0 ]]; then
	 MCOPT="-nevents=$NEVENTS -firstevent=$FIRSTEVENT $MCOPT"
fi
echo "Analysing MC file for run $RUNNUM"
echo "$EVNDISPSYS/bin/evndisp $MCOPT $MODEL3D $FROGS" &> $ODIR/$ONAME.log
$EVNDISPSYS/bin/evndisp $MCOPT $MODEL3D $FROGS $AMPCORR &>> $ODIR/$ONAME.log

# remove temporary files
cp -f -v $DDIR/$ONAME.root $ODIR/$ONAME.root
chmod g+w $ODIR/$ONAME.root
chmod g+w $ODIR/$ONAME.log
chmod g+w $ODIR/$ONAME.tzero.log
chmod -R g+w $ODIR/Calibration
rm -f -v $DDIR/$ONAME.root
rm -f -v $VBF_FILE

echo "EVNDISP output root file written to $ODIR/$ONAME.root"
echo "EVNDISP log file written to $ODIR/$ONAME.log"

exit
