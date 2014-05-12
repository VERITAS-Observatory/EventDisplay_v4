#!/bin/bash
# script to extract simulation VBF files before running them through evndisp
# Scott Griffiths

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS

# parameters replaced by parent script using sed
ZA=ZENITHANGLE
WOB=DECIMALWOBBLE
WOG=INTEGERWOBBLE
NOISE=NOISELEVEL
EPOCH=ARRAYEPOCH
ATM=ATMOSPHERE
PARTICLE=PARTICLETYPE
SIMDIR=DATADIR
ODIR=OUTPUTDIR
FILENUM=FILENUMBER
SIMTYPE=SIMULATIONTYPE

# Input files (observe that these might need some adjustments)
if [[ $SIMTYPE = "GRISU" ]]; then
    if [[ $EPOCH == "V4" ]]; then
        if [[ $PARTICLE == "1" ]]; then
            VBFNAME="$Oct2012_oa_ATM${ATM}_${ZA}deg_${WOG}"
            RUN_IDS=( "wobb" )
        elif [[ $PARTICLE == "14" ]]; then
            VBFNAME="proton_${ZA}deg_750m_wobble${WOB}_2008_2009_"
            RUN_IDS=( "8${ZA}0" "9${ZA}0" "9${ZA}5" )
        fi
    elif [[ $EPOCH == "V5" ]]; then
        if [[ $PARTICLE == "1" ]]; then
            VBFNAME="gamma_V5_Oct2012_newArrayConfig_20121027_v420_ATM${ATM}_${ZA}deg_${WOG}"
            RUN_IDS=( "wobb" )
        elif [[ $PARTICLE == "14" ]]; then
            VBFNAME="proton_${ZA}deg_w${WOB}_"
            RUN_IDS=( $FILENUM )
        elif [[ $PARTICLE == "402" ]]; then
            VBFNAME="helium_${ZA}deg_w${WOB}_"
            RUN_IDS=( 800 )
        fi
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
            RUN_IDS=( "wobb" )
        elif [[ $PARTICLE == "14" ]]; then
            VBFNAME="proton_${ZA}deg_w${WOB}_"
            RUN_IDS=( $FILENUM )
        elif [[ $PARTICLE == "402" ]]; then
            VBFNAME="helium_${ZA}deg_w${WOB}_"
            RUN_IDS=( 800 )
        fi
    fi
elif [[ $SIMTYPE = "CARE" ]]; then
    [[ $PARTICLE == "1" ]]  && VBFNAME="gamma_${ZA}deg_750m_${WOB}wob_${NOISE}mhz_up_ATM${ATM}_part0"
    [[ $PARTICLE == "2" ]]  && VBFNAME="electron_${ZA}deg_noise${NOISE}MHz___"
    [[ $PARTICLE == "14" ]] && VBFNAME="proton_${ZA}deg_noise${NOISE}MHz___"
fi

# temporary (scratch) directory
if [[ -n $TMPDIR ]]; then
    TEMPDIR="$TMPDIR/MC_VBF"
else
    TEMPDIR="$VERITAS_USER_DATA_DIR/TMPDIR/MC_VBF"
fi
mkdir -p $TEMPDIR

# loop over simulation files
for RUN_ID in ${RUN_IDS[@]}; do
    if [[ $SIMTYPE = "GRISU" ]]; then
        VBF_FILE="$VBFNAME$RUN_ID.vbf"
    elif [[ $SIMTYPE = "CARE" ]]; then
        VBF_FILE="$VBFNAME.cvbf"
    fi
    echo "Now processing $VBF_FILE"

    # unzip vbf file to local scratch directory
    if [ ! -e "$TEMPDIR/$VBF_FILE" ]; then
        if [[ -e "$SIMDIR/$VBF_FILE.gz" ]]; then
            echo "Copying $SIMDIR/${VBF_FILE}.gz to $TEMPDIR"
            cp -f "$SIMDIR/$VBF_FILE.gz" $TEMPDIR/
            echo " (vbf file copied, was gzipped)"
            gunzip -f -v "$TEMPDIR/$VBF_FILE.gz"
        elif [[ -e "$SIMDIR/$VBF_FILE.bz2" ]]; then
            echo "Copying $SIMDIR/$VBF_FILE.bz2 to $TEMPDIR"
            cp -f "$SIMDIR/$VBF_FILE.bz2" $TEMPDIR/
            echo " (vbf file copied, was bzipped)"
            bunzip2 -f -v "$TEMPDIR/$VBF_FILE.bz2"
        elif [[ -e "$SIMDIR/$VBF_FILE" ]]; then
            echo "Copying $VBF_FILE to $TEMPDIR"
            cp -f "$SIMDIR/$VBF_FILE" $TEMPDIR/
        fi
    fi
    
    # check that the uncompressed vbf file exists
    if [ ! -e "$TEMPDIR/$VBF_FILE" ]; then
        echo "No source file found: $TEMPDIR/$VBF_FILE"
        echo "$SIMDIR/$VBF_FILE*"
        exit 1
    fi
    VBF_FILE="$TEMPDIR/$VBF_FILE"
done

exit

    