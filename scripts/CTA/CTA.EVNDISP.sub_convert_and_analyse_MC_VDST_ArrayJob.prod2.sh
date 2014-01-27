#!/bin/sh
#
# script to convert sim_tel output files and then run eventdisplay analysis
#
# PROD2
#
# Author: Gernot Maier
#
#######################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]
then
   echo
   echo "./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob <sub array list> <list of simtelarray files> <particle> <data set> [keep simtel.root files (default off=0)] [log file directory counter] [qsub options] [TRIGGER MASK DIRECTORY]"
   echo
   echo "CTA PROD2 ANALYSIS"
   echo
   echo "  <sub array list>          text file with list of subarray IDs"
   echo
   echo "  <particle>                gamma_onSource , gamma_diffuse, proton , electron (helium, ...)"
   echo
   echo "  <data set>                e.g. cta-ultra3, ISDC3700m, ..."
   echo ""
   echo "NOTE: HARDWIRED FILE NAMES IN QSUB SCRIPTS !!"
   echo ""
   echo "  [keep DST.root files]  keep and copy converted simtel files (DST files) to output directory (default off=0)"
   echo ""
   echo "  [TRIGGER MASK DIRECTORY] directory with trigger mask file (naming!)"
   echo ""
   echo " output will be written to: CTA_USER_DATA_DIR/analysis/<subarray>/<particle>/ "
   echo ""
   echo ""
   echo " REMINDER:"
   echo "  - compile CTA converter before starting the scripts (make CTA.convert_hessio_to_VDST)"
   echo "  - make sure that your evndisp installation runs with the same parameters as hessio was compiled with "
   echo "      build_all ultra: -DCTA -DCTA_ULTRA "
   echo "      build_all max:   -DCTA_MAX"
   exit
fi

############################################################################
# RUN PARAMETERS
# prod2 no double pass cleaning
# ARRAYCUTS="EVNDISP.prod2-noDoublepass.reconstruction.runparameter"
# prod2 TIMENEXTNEIGHBOUR cleaning
# ARRAYCUTS="EVNDISP.prod2.TIMENEXTNEIGHBOUR.reconstruction.runparameter"
# PPOPT="\"-NNcleaninginputcard /lustre/fs9/group/cta/users/maierg/CTA/analysis/AnalysisData/TIMENEXTNEIGHBOUR/Evtdisp.rc\""
# prod2 default file
ARRAYCUTS="EVNDISP.prod2.reconstruction.runparameter"
PPOPT=""
############################################################################

ARRAY=$1
RUNLIST=$2
PART=$3
DSET=$4
KEEP=0
if [ -n "$5" ]
then
   KEEP=$5
fi
FLL="0"
if [ -n "$6" ]
then
  FLL="$6"
fi
TRGMASKDIR="FALSE"
if [ -n $8 ]
then
  TRGMASKDIR="$8"
fi
QSUBOPT=""
if [ -n $7 ]
then
   QSUBOPT="$7"
fi
QSUBOPT=${QSUBOPT//_X_/ } 
QSUBOPT=${QSUBOPT//_M_/-} 

# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

# get run list and number of runs
if [ ! -e $RUNLIST ]
then
  echo "list of simtelarray files not found: $RUNLIST"
  exit
fi
NRUN=`wc -l $RUNLIST | awk '{print $1}'`
RUNFROMTO="1-$NRUN"

#########################################
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`

# output directory for shell scripts
SHELLDIR=$CTA_USER_DATA_DIR"/queueShellDir/"
mkdir -p $SHELLDIR

# skeleton script
FSCRIPT="CTA.EVNDISP.qsub_convert_and_analyse_MC_VDST_ArrayJob.prod2"

# log files
#QLOG=$CTA_USER_LOG_DIR/$DATE/EVNDISP-$PART-$DSET/
#mkdir -p $QLOG
QLOG="/dev/null"

# pedestals
# PEDFIL="$CTA_USER_DATA_DIR/analysis/AnalysisData/prod2-Leoncito/Calibration/Leoncito.peds.root"
PEDFIL="$CTA_USER_DATA_DIR/analysis/AnalysisData/prod2-Aar/Calibration/Aar.peds.root"

echo "submitting $RUNFROMTO"

FNAM="$SHELLDIR/EV-$DSET-$PART-$FLL"

LIST=`awk '{printf "%s ",$0} END {print ""}' $ARRAY`

sed -e "s|SIMTELLIST|$RUNLIST|" \
    -e "s|PAAART|$PART|" \
    -e "s!ARRAY!$LIST!" \
    -e "s|KEEEEEEP|$KEEP|" \
    -e "s|ARC|$ARRAYCUTS|" \
    -e "s|DATASET|$DSET|" \
    -e "s|FLL|$FLL|" \
    -e "s|PPPP|$PEDFIL|" \
    -e "s!UUUU!$PPOPT!" \
    -e "s|TRIGGGG|$TRGMASKDIR|" $FSCRIPT.sh > $FNAM.sh

chmod u+x $FNAM.sh
echo $FNAM.sh

qsub $QSUBOPT -t $RUNFROMTO:1  -l h_cpu=47:29:00 -l os=sl6 -l tmpdir_size=10G -l h_vmem=4G -V -o $QLOG -e $QLOG "$FNAM.sh" 

echo "writing shell script to $FNAM.sh"
echo "writing queue log and error files to $QLOG"

exit
