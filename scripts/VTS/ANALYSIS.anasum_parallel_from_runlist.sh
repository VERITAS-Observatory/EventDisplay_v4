#!/bin/bash
# script to analyse data files with anasum (parallel analysis) from a simple run list

# EventDisplay version
EDVERSION=`$EVNDISPSYS/bin/anasum --version | tr -d .`

if [[ $# < 4 ]]; then
# begin help message
echo "
ANASUM parallel data analysis: submit jobs using a simple run list

ANALYSIS.anasum_parallel_from_runlist.sh <run list> <output directory> <cut set> <background model> [run parameter file] [mscw directory] [sim type] [method] [force atmosphere] 

required parameters:

    <run list>              simple runlist with a single run number per line
        
    <output directory>      anasum output files are written to this directory
                        
    <cut set>               hardcoded cut sets predefined in the script
                            (e.g., BDTmoderate2tel, BDTsoft2tel, BDThard3tel etc.)
    
    <background model>      background model
                            (RE = reflected region, RB = ring background, IGNOREACCEPTANCE = RE without ACCEPTANCE)
    
optional parameters:

    [run parameter file]    anasum run parameter file (located in 
                            \$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/;
                            default is ANASUM.runparameter)

    [mscw directory]        directory containing the mscw.root files.
			    Default: $VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION

    [sim type]              use IRFs derived from this simulation type (GRISU-SW6 or CARE_June1702)
			    Default: CARE_June1702

    [method]                reconstruction method: GEO, DISP, FROGS.
			    Default: GEO

    [force atmosphere]	    use EAs generated with this atmospheric model (61 or 62).
			    Default: Atmosphere determined from run date for each run.				
			    Attention: Must use the same atmospere for EAs as was used for the lookup tables in the mscw_energy stage!

IMPORTANT! Run ANALYSIS.anasum_combine.sh once all parallel jobs have finished!

--------------------------------------------------------------------------------
"
#end help message
exit
fi

###########################
# IRFs
IRFVERSION=`$EVNDISPSYS/bin/mscw_energy --version | tr -d . | sed -e 's/[a-zA-Z]*$//'`
AUXVERSION="auxv01"

# Run init script
bash $(dirname "$0")"/helper_scripts/UTILITY.script_init.sh"
[[ $? != "0" ]] && exit 1

# Parse command line arguments
RLIST=$1
ODIR=$2
CUTS=$3
BACKGND=$4
[[ "$5" ]] && RUNP=$5  || RUNP="ANASUM.runparameter"
[[ "$6" ]] && INDIR=$6 || INDIR="$VERITAS_USER_DATA_DIR/analysis/Results/$EDVERSION/"
[[ "$7" ]] && SIMTYPE=$7 || SIMTYPE="DEFAULT"
[[ "$8" ]] && METH=$8 || METH="GEO"
[[ "$9" ]] && FORCEDATMO=$9 

SIMTYPE_DEFAULT_V4="GRISU"
SIMTYPE_DEFAULT_V5="GRISU"
SIMTYPE_DEFAULT_V6="CARE_June1702"
SIMTYPE_DEFAULT_V6redHV="CARE_RedHV"

# cut definitions (note: VX to be replaced later in script)
if [[ $CUTS == superhard ]]; then
    CUT="NTel3-PointSource-SuperHard"
elif [[ $CUTS == super ]]; then
    CUT="NTel2-PointSource-SuperSoftSpectrum"
elif [[ $CUTS == moderateopen ]]; then
    CUT="NTel2-PointSource-ModerateOpen"
elif [[ $CUTS == softopen ]]; then
    CUT="NTel2-PointSource-SoftOpen"
elif [[ $CUTS == hardopen ]]; then
    CUT="NTel2-PointSource-HardOpen"
elif [[ $CUTS == softExt ]]; then
    CUT="NTel2-ExtendedSource-Soft"
elif [[ $CUTS == soft2tel ]]; then
    CUT="NTel2-PointSource-Soft"
elif [[ $CUTS = moderate2tel ]]; then
    CUT="NTel2-PointSource-Moderate"
elif [[ $CUTS = moderateExt2tel ]]; then
    CUT="NTel2-ExtendedSource-Moderate"
elif [[ $CUTS = moderate3tel ]]; then
    CUT="NTel3-PointSource-Moderate"
elif [[ $CUTS = hard2tel ]]; then
    CUT="NTel2-PointSource-Hard"
elif [[ $CUTS = hard3tel ]]; then
    CUT="NTel3-PointSource-Hard"
elif [[ $CUTS = hardExt2tel ]]; then
    CUT="NTel2-ExtendedSource-Hard"
elif [[ $CUTS = frogs ]]; then
    CUT="FROGS_NTel2_001-003-005CU_index2.5"
elif [[ $CUTS = BDTmoderate2tel ]]; then
    CUT="NTel2-PointSource-Moderate-TMVA-BDT"
elif [[ $CUTS = BDTsoft2tel ]]; then
    CUT="NTel2-PointSource-Soft-TMVA-BDT"
elif [[ $CUTS = BDThard2tel ]]; then 
    CUT="NTel2-PointSource-Hard-TMVA-BDT"
elif [[ $CUTS = BDTmoderate2telweak ]]; then
    CUT="NTel2-PointSource-Moderate-TMVA-BDT-weak"
elif [[ $CUTS = BDThard2telweak ]]; then
    CUT="NTel2-PointSource-Hard-TMVA-BDT-weak"
elif [[ $CUTS = BDTmoderate3tel ]]; then
    CUT="NTel3-PointSource-Moderate-TMVA-BDT"
elif [[ $CUTS = BDThard3tel ]]; then
    CUT="NTel3-PointSource-Hard-TMVA-BDT"
elif [[ $CUTS = BDTmoderateExt2tel ]]; then
    CUT="NTel2-ExtendedSource-Moderate-TMVA-BDT"
elif [[ $CUTS = BDTsoftExt2tel ]]; then
    CUT="NTel2-ExtendedSource-Soft-TMVA-BDT"
elif [[ $CUTS = BDThardExt2tel ]]; then
    CUT="NTel2-ExtendedSource-Hard-TMVA-BDT"
elif [[ $CUTS = BDTmoderateExt3tel ]]; then
    CUT="NTel3-ExtendedSource-Moderate-TMVA-BDT"
elif [[ $CUTS = BDThardExt3tel ]]; then
    CUT="NTel3-ExtendedSource-Hard-TMVA-BDT"
elif [[ $CUTS = NTel2ModeratePre ]]; then
    CUT="NTel2-PointSource-Moderate-TMVA-Preselection"
elif [[ $CUTS = NTel2SoftPre ]]; then
    CUT="NTel2-PointSource-Soft-TMVA-Preselection"
else
    echo "ERROR: unknown cut definition: $CUTS"
    exit 1
fi
CUTFILE="ANASUM.GammaHadron-Cut-${CUT}.dat"
EFFAREA="effArea-${IRFVERSION}-${AUXVERSION}-SX-Cut-${CUT}-${METH}-VX-ATMXX-TX.root"

# remove PointSource and ExtendedSource string from cut file name for radial acceptances names
if [[ $CUT == *PointSource-* ]] ; then
    CUTRADACC=${CUT/-PointSource-/"-"}
    echo $CUTRACACC
elif [[ $CUT == *ExtendedSource-* ]]; then
    CUTRADACC=${CUT/-ExtendedSource-/"-"}
    echo $CUTRADACC
fi

RADACC="radialAcceptance-${IRFVERSION}-${AUXVERSION}-SX-Cut-${CUTRADACC}-${METH}-VX-TX.root"

echo $CUTFILE
echo $EFFAREA
echo $RADACC

# background model parameters
if [[ "$BACKGND" == *RB* ]]; then
    BM="RB"
    BMPARAMS="0.6 20"
elif [[ "$BACKGND" == *IGNOREACCEPTANCE* ]]; then
    BM="RE"
    BMPARAMS="0.1 2 6"
    RADACC="IGNOREACCEPTANCE"
elif [[ "$BACKGND" == *RE* ]]; then
    BM="RE"
    BMPARAMS="0.1 2 6"
else
    echo "ERROR: unknown background model: $BACKGND"
    echo "Allowed values are: RE, RB"
    exit 1
fi

# Check that run list exists
if [[ ! -f "$RLIST" ]]; then
    echo "Error, simple runlist $RLIST not found, exiting..."
    exit 1
fi

# Check that run parameter file exists
if [[ "$RUNP" == `basename $RUNP` ]]; then
    RUNP="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/$RUNP"
fi
if [ ! -f "$RUNP" ]; then
    echo "Error, anasum run parameter file not found, exiting..."
    exit 1
fi

# directory for run scripts
DATE=`date +"%y%m%d"`
LOGDIR="$VERITAS_USER_LOG_DIR/$DATE/ANASUM.ANADATA"
echo -e "Log files will be written to:\n $LOGDIR"
mkdir -p $LOGDIR

# output directory for anasum products
echo -e "Output files will be written to:\n $ODIR"
mkdir -p $ODIR

#########################################
# make anasum run list
ANARUNLIST="$ODIR/$CUTS.anasum.dat"
rm -f $ANARUNLIST
echo "anasum run list: $ANARUNLIST"

# run list header
echo "* VERSION 6" >> $ANARUNLIST
echo "" >> $ANARUNLIST

RUNS=`cat $RLIST`

for RUN in ${RUNS[@]}; do
    # get array epoch, atmosphere and telescope combination for this run
    RUNINFO=`$EVNDISPSYS/bin/printRunParameter $INDIR/$RUN.mscw.root -runinfo`
    EPOCH=`echo $RUNINFO | awk '{print $(1)}'`
    MAJOREPOCH=`echo $RUNINFO | awk '{print $(2)}'`
    ATMO=${FORCEDATMO:-`echo $RUNINFO | awk '{print $(3)}'`}
    if [[ $ATMO == *error* ]]; then
       echo "error finding atmosphere; skipping run $RUN"
       continue
    fi
    OBSL=$(echo $RUNINFO | awk '{print $4}')
    TELTOANA=`echo $RUNINFO | awk '{print "T"$(5)}'`
    if [[ $EPOCH == *"V4"* ]] || [[ $EPOCH == *"V5"* ]]; then
        ATMO=${ATMO/6/2}
    fi
    if [[ $SIMTYPE == "DEFAULT" ]]; then
        if [[ $EPOCH == *"V4"* ]]; then
            REPLACESIMTYPEEff=${SIMTYPE_DEFAULT_V4}
            REPLACESIMTYPERad=${SIMTYPE_DEFAULT_V4}
        elif [[ $EPOCH == *"V5"* ]]; then
            REPLACESIMTYPEEff=${SIMTYPE_DEFAULT_V5}
            REPLACESIMTYPERad=${SIMTYPE_DEFAULT_V5}
        elif [[ $EPOCH == *"V6"* ]] && [[ $OBSL == "obsLowHV" ]]; then
            REPLACESIMTYPEEff=${SIMTYPE_DEFAULT_V6redHV}
            REPLACESIMTYPERad=${SIMTYPE_DEFAULT_V6}
        else
            REPLACESIMTYPEEff=${SIMTYPE_DEFAULT_V6}
            REPLACESIMTYPERad=${SIMTYPE_DEFAULT_V6}
        fi
     else
        REPLACESIMTYPEEff=${SIMTYPE}
        REPLACESIMTYPERad=${SIMTYPE}
     fi

    echo "RUN $RUN at epoch $EPOCH and atmosphere $ATMO (Telescopes $TELTOANA SIMTYPE $REPLACESIMTYPEEff $REPLACESIMTYPERad)"
    echo "File $INDIR/$RUN.mscw.root"
    
    # do string replacements
    EFFAREARUN=${EFFAREA/VX/$EPOCH}
    EFFAREARUN=${EFFAREARUN/TX/$TELTOANA}
    EFFAREARUN=${EFFAREARUN/XX/$ATMO}
    EFFAREARUN=${EFFAREARUN/SX/$REPLACESIMTYPEEff}
    if [ "$RADACC" != "IGNOREACCEPTANCE" ]; then 
        RADACCRUN=${RADACC/VX/$MAJOREPOCH}
        RADACCRUN=${RADACCRUN/TX/$TELTOANA}
        RADACCRUN=${RADACCRUN/SX/$REPLACESIMTYPERad}
    else
        RADACCRUN=$RADACC
    fi
    
    # write line to file
    echo "* $RUN $RUN 0 $CUTFILE $BM $EFFAREARUN $BMPARAMS $RADACCRUN" >> $ANARUNLIST
    echo "* $RUN $RUN 0 $CUTFILE $BM $EFFAREARUN $BMPARAMS $RADACCRUN"

done

# submit the job
SUBSCRIPT="$EVNDISPSYS/scripts/VTS/ANALYSIS.anasum_parallel"
$SUBSCRIPT.sh $ANARUNLIST $INDIR $ODIR $RUNP

exit
