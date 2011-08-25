#!/bin/sh
#
# script to convert sim_tel output files and then run eventdisplay analysis
#
# Revision $Id: mm_sub_evndisp_CTA.sh,v 1.1.2.1.2.2.2.2 2011/04/06 11:59:25 gmaier Exp $
#
# Author: Gernot Maier
#
#######################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]
then
   echo "./CTA.EVNDISP.sub_convert_and_analyse_MC_VDST.sh <sub array> <list of simtelarray files> <particle> [keep simtel.root files (default off=0)] [method=GEO/LL] "
   echo
   echo "  <sub array>               sub array from prod1 (e.g. E)"
   echo "  <particle>                gamma_onSource, gamma_diffuse, proton, electron (helium, ...)"
   echo ""
   echo "  [keep simtel.root files]  keep and copy converted simtel files to output directory (default off=0)"
   echo "  [method]                  reconstruction method (default=GEO)"
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
# FIXED PARAMETERS
ARRAYCUTS="EVNDISP.reconstruction.runparameter"
############################################################################

ARRAY=$1
BLIST=$2
PART=$3
KEEP=0
if [ -n "$4" ]
then
   KEEP=$4
fi
MET="GEO"
if [ -n "$5" ]
then
  MET=$5
fi

# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

############################################################################

FILES=`cat $BLIST`

#########################################
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`

# output directory for shell scripts
SHELLDIR=$CTA_USER_LOG_DIR"/queueShellDir/"
mkdir -p $SHELLDIR

# skeleton script
FSCRIPT="CTA.EVNDISP.qsub_convert_and_analyse_MC_VDST"

Z=0
N=100

# loop over all files in files loop
for AFIL in $FILES
do
   QLOG=$CTA_USER_LOG_DIR/$DATE/conv$N/
   mkdir -p $QLOG
   
   echo "submitting $AFIL ($LDIR)"

   FNAM="$SHELLDIR/EVN.ConvAna-$ARRAY-$PART"

   sed -e "s|SIMTELFILE|$AFIL|" $FSCRIPT.sh > $FNAM-1.sh
   sed -e "s|PAAART|$PART|" $FNAM-1.sh > $FNAM-2.sh
   rm -f $FNAM-1.sh
   sed -e "s|ARRAY|$ARRAY|" $FNAM-2.sh > $FNAM-3.sh
   rm -f $FNAM-2.sh
   sed -e "s|KEEEEEEP|$KEEP|" $FNAM-3.sh > $FNAM-4.sh
   rm -f $FNAM-3.sh
   sed -e "s|ARC|$ARRAYCUTS|" $FNAM-4.sh > $FNAM-5.sh
   rm -f $FNAM-4.sh
   sed -e "s|MEEET|$MET|" $FNAM-5.sh > $FNAM.sh
   rm -f $FNAM-5.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

   qsub -l h_cpu=04:29:00 -l tmpdir_size=10G -l h_vmem=3G -V -o $QLOG -e $QLOG "$FNAM.sh"
   echo "writing shell script to $FNAM.sh"
   echo "writing queue log and error files to $QLOG"

   let "Z = $Z + 1"

   if [ $Z -gt 200 ]
   then
      let "N = $N + 1"
      Z=0
   fi
      
done

exit
