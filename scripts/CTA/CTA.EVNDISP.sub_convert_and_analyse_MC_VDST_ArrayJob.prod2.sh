#!/bin/sh
#
# script to convert sim_tel output files and then run eventdisplay analysis
#
# Author: Gernot Maier
#
#######################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]
then
   echo
   echo "./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST_ArrayJob <sub array list> <list of simtelarray files> <particle> <data set> [keep simtel.root files (default off=0)] [log file directory counter] [TRIGGER MASK DIRECTORY]"
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
ARRAYCUTS="EVNDISP.prod2.reconstruction.runparameter"
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
if [ -n $7 ]
then
  TRGMASKDIR="$7"
fi

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
QLOG=$CTA_USER_LOG_DIR/$DATE/EVNDISP-$PART-$DSET/
#mkdir -p $QLOG
QLOG="/dev/null"

# pedestals
echo $DSET
if [[ $DSET == *Leoncito* ]] || [[ $DSET == *SAC* ]]
then
   echo "Leoncito / SAC"
   PEDFIL="$CTA_USER_DATA_DIR/analysis/AnalysisData/prod2-Leoncito/Calibration/Leoncito.peds.root"
elif [[ $DSET == *Aar* ]] || [[ $DSET = *NorthSite-BB* ]] || [[ $DSET == *US* ]] || [[ $DSET == *SPM* ]] || [[ $DSET == *Tenerife* ]]
then
   echo "Aar"
# use always Leoncito peds - should be exactly the same
#   PEDFIL="$CTA_USER_DATA_DIR/analysis/AnalysisData/prod2-Aar/Calibration/Aar.peds.root"
   PEDFIL="$CTA_USER_DATA_DIR/analysis/AnalysisData/prod2-Leoncito/Calibration/Leoncito.peds.root"
else
   echo "error: data set not known (not Leoncito, SAC or Aar)"
   echo "       (no ped files available)"
   exit
fi


echo "submitting $RUNFROMTO"

FNAM="$SHELLDIR/EC-$DSET-$PART-$FLL"

sed -e "s|SIMTELLIST|$RUNLIST|" $FSCRIPT.sh > $FNAM-1.sh
sed -e "s|PAAART|$PART|" $FNAM-1.sh > $FNAM-2.sh
rm -f $FNAM-1.sh
LIST=`awk '{printf "%s ",$0} END {print ""}' $ARRAY`
sed -e "s!ARRAY!$LIST!" $FNAM-2.sh > $FNAM-3.sh
rm -f $FNAM-2.sh
sed -e "s|KEEEEEEP|$KEEP|" $FNAM-3.sh > $FNAM-4.sh
rm -f $FNAM-3.sh
sed -e "s|ARC|$ARRAYCUTS|" $FNAM-4.sh > $FNAM-5.sh
rm -f $FNAM-4.sh
sed -e "s|DATASET|$DSET|" $FNAM-5.sh > $FNAM-7.sh
rm -f $FNAM-5.sh
sed -e "s|FLL|$FLL|" $FNAM-7.sh > $FNAM-8.sh
rm -f $FNAM-7.sh
sed -e "s|PPPP|$PEDFIL|" $FNAM-8.sh > $FNAM-9.sh
rm -f $FNAM-8.sh
sed -e "s|TRIGGGG|$TRGMASKDIR|" $FNAM-9.sh > $FNAM.sh
rm -f $FNAM-9.sh

chmod u+x $FNAM.sh
echo $FNAM.sh

qsub -t $RUNFROMTO:1  -l h_cpu=47:29:00 -l os="sl*" -l tmpdir_size=10G -l h_vmem=4G -V -o $QLOG -e $QLOG "$FNAM.sh"

echo "writing shell script to $FNAM.sh"
echo "writing queue log and error files to $QLOG"

exit
