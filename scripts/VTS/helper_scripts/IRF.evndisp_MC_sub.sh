#!/bin/bash
# script to run evndisp for simulations on one of the cluster nodes (VBF)

# set observatory environmental variables
source $TRUNK/setObservatory.sh VTS

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
USEMODEL3D=USEMODEL3DMETHOD
USEFROGS=FROGSFROGS
MSCWDIR=FROGSMSCWDIR
NEVENTS=FROGSEVENTS
TELTOANA="1234"

# Output file name
ONAME="$RUNNUM"

if [[ $NEVENTS > 0 ]]; then
    ITER=$((SGE_TASK_ID - 1))
    FIRSTEVENT=$(($ITER * $NEVENTS))
    # Output file name
    ONAME="${RUNNUM}_$ITER"
    echo -e "ITER $ITER NEVENTS $NEVENTS FIRSTEVENT $FIRSTEVENT"
fi

#################################
# detector configuration and cuts

echo "Using run parameter file $ACUTS"
# no disp, long integration window
# ACUTS="EVNDISP.reconstruction.runparameter"
# disp, long integration window
# ACUTS="EVNDISP.reconstruction.runparameter.DISP"
# no disp, short integration window
# ACUTS="EVNDISP.reconstruction.runparameter.SumWindow6-noDISP"
# disp, short integration window
# ACUTS="EVNDISP.reconstruction.runparameter.SumWindow6-DISP"


DEAD="EVNDISP.validchannels.dat"
PEDLEV="16."
# LOWPEDLEV="8."
LOWPEDLEV="16."

if [[ ${SIMTYPE:0:5} == "GRISU" ]]; then
    # Input files (observe that these might need some adjustments)
    if [[ $EPOCH == "V4" ]]; then
        if [[ $PARTICLE == "1" ]]; then
           if [[ $ATM == "21" ]]; then
            VBFNAME="Oct2012_oa_ATM21_${ZA}deg_${WOG}"
           else
            VBFNAME="gamma_V4_Oct2012_SummerV4ForProcessing_20130611_v420_ATM${ATM}_${ZA}deg_${WOG}"
           fi
        elif [[ $PARTICLE == "14" ]]; then
            VBFNAME="proton_${ZA}deg_750m_wobble${WOB}_2008_2009_"
        fi
        NOISEFILE="$OBS_EVNDISP_ANA_DIR/NOISE/NOISE$NOISE.grisu"
        echo "Noise File: $NOISEFILE"
    elif [[ $EPOCH == "V5" ]]; then
        if [[ $PARTICLE == "1" ]]; then
            VBFNAME="gamma_V5_Oct2012_newArrayConfig_20121027_v420_ATM${ATM}_${ZA}deg_${WOG}"
        elif [[ $PARTICLE == "14" ]]; then
            VBFNAME="proton_${ZA}deg_w${WOB}_"
        elif [[ $PARTICLE == "402" ]]; then
            VBFNAME="helium_${ZA}deg_w${WOB}_"
        fi
        NOISEFILE="$OBS_EVNDISP_ANA_DIR/NOISE/NOISE$NOISE.grisu"
        echo "Noise File: $NOISEFILE"
    elif [[ $EPOCH == "V6" ]]; then
        if [[ $PARTICLE == "1" ]]; then
            VBFNAME="gamma_V6_Upgrade_20121127_v420_ATM${ATM}_${ZA}deg_${WOG}"
            if [[ $ATM == "21-redHV" ]]; then
                VBFNAME="gamma_V6_Upgrade_ReducedHV_20121211_v420_ATM21_${ZA}deg_${WOG}"
            elif [[ $ATM == "21-UV" ]]; then
                VBFNAME="gamma_V6_Upgrade_UVfilters_20121211_v420_ATM21_${ZA}deg_${WOG}"
            elif [[ $ATM == "21-SNR" ]]; then
                VBFNAME="gamma_V6_201304_SN2013ak_v420_ATM21_${ZA}deg_${WOG}"
            fi
        elif [[ $PARTICLE == "14" ]]; then
            VBFNAME="proton_${ZA}deg_w${WOB}_"
        elif [[ $PARTICLE == "402" ]]; then
            VBFNAME="helium_${ZA}deg_w${WOB}_"
        fi
        NOISEFILE="$OBS_EVNDISP_ANA_DIR/NOISE/NOISE${NOISE}_20120827_v420.grisu"
        echo "Noise File: $NOISEFILE"
    fi
elif [ ${SIMTYPE:0:4} == "CARE" ]; then
    # input files (observe that these might need some adjustments)
    [[ $PARTICLE == "1" ]]  && VBFNAME="gamma_${ZA}deg_750m_${WOB}wob_${NOISE}mhz_up_ATM${ATM}_part0"
    [[ $PARTICLE == "2" ]]  && VBFNAME="electron_${ZA}deg_noise${NOISE}MHz___"
    [[ $PARTICLE == "14" ]] && VBFNAME="proton_${ZA}deg_noise${NOISE}MHz___"

fi
# detector configuration
[[ $EPOCH == "V4" ]] && CFG="EVN_V4_Oct2012_oldArrayConfig_20130428_v420.txt"
[[ $EPOCH == "V5" ]] && CFG="EVN_V5_Oct2012_newArrayConfig_20121027_v420.txt"
[[ $EPOCH == "V6" ]] && CFG="EVN_V6_Upgrade_20121127_v420.txt"
    

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
    $TRUNK/bin/evndisp -runmode=1 -sourcetype=2 -epoch $EPOCH -sourcefile $VBF_FILE -runnumber=$RUNNUM -calibrationsumfirst=0 -calibrationsumwindow=20 -donotusedbinfo -calibrationdirectory $ODIR &> $ODIR/$RUNNUM.ped.log
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
    echo "$TRUNK/bin/evndisp $MCOPT" &> $ODIR/$RUNNUM.tzero.log
    $TRUNK/bin/evndisp $MCOPT &>> $ODIR/$RUNNUM.tzero.log
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
if [[ $NEVENTS > 0 ]]; then
	 MCOPT="-nevents=$NEVENTS -firstevent=$FIRSTEVENT $MCOPT"
fi
echo "Analysing MC file for run $RUNNUM"
echo "$TRUNK/bin/evndisp $MCOPT $MODEL3D $FROGS" &> $ODIR/$ONAME.log
$TRUNK/bin/evndisp $MCOPT $MODEL3D $FROGS &>> $ODIR/$ONAME.log

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
