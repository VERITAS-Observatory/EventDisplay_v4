#!/bin/sh
#
# script to analyse MC files with lookup tables
#
# Revision $Id: analyse_MC_CTA.sh,v 1.1.2.1.2.2.2.2 2011/02/14 16:19:27 gmaier Exp $
#
# Author: Gernot Maier
#


if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ] && [ ! -n "$5" ]
then
   echo
   echo "CTA.MSCW_ENERGY.sub_analyse_MC.sh <tablefile> <recid> <subarray list> <particle> <data set> [wildcard]"
   echo
   echo "  <tablefile>     table file name (without .root)"
   echo "                  expected file name: xxxxxx-SUBARRAY.root; SUBARRAY is added by this script"
   echo "  <recid>         reconstruction ID"
   echo "  <subarraylist > text file with list of subarray IDs"
   echo "  <particle>      gamma_onSource / gamma_cone10 / electron / proton / helium"
   echo "  <data set>      e.g. ultra, ISDC3700m, ..."
   echo
   echo "optional (for a huge amount of MC files):"
   echo "  [wildcard]     used in the < CTA.MSCW_ENERGY.subParallel_analyse_MC.sh > script"
   echo
   exit
fi

#########################################
# input parameters
TABLE=$1
RECID=$2
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $3`

PART=$4
METH="LL"
WC=""
DSET="$5"
if [ -n "$6" ]
then
   WC=$6
fi

#########################################
# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

#########################################
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`
QLOG=$CTA_USER_LOG_DIR/$DATE/
mkdir -p $QLOG

# output directory for shell scripts
SHELLDIR=$CTA_USER_LOG_DIR"/queueShellDir/"
mkdir -p $SHELLDIR

#########################################
#loop over all arrays
#########################################
for SUBAR in $VARRAY
do
   echo "STARTING ARRAY $SUBAR"

#########################################
# input files
#IFIL="$CTA_DATA_DIR/analysis/$SUBAR/$PART/$WC"
   IFIL="$CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$SUBAR/$PART/$WC"

# check if input files exist
   IFILN=`ls -1 $IFIL*.root | wc -l`
   if [ $IFILN -eq 0 ]
   then
     echo "No input files in $IFIL"
     echo "exiting..."
     exit
   fi
   echo "FOUND $IFILN input files in $IFIL"


# output file name for mscw_energy
   TFIL=$PART$NC"."$SUBAR"_ID"$RECID".mscw"
   if [ ${#WC} -gt 0 ]
   then 
      TFIL=$PART$NC"."$SUBAR"_ID"$RECID"-$WC.mscw"
   fi

# skeleton script
   FSCRIPT="CTA.MSCW_ENERGY.qsub_analyse_MC"

   FNAM="$SHELLDIR/MSCW.ana-ID$RECID-$PART-array$SUBAR"

   sed -e "s|TABLEFILE|$TABLE|" $FSCRIPT.sh > $FNAM-1.sh
   sed -e "s|IIIIFIL|$IFIL|" $FNAM-1.sh > $FNAM-2.sh
   rm -f $FNAM-1.sh
   sed -e "s|TTTTFIL|$TFIL|" $FNAM-2.sh > $FNAM-3.sh
   rm -f $FNAM-2.sh
   sed -e "s|RECONSTRUCTIONID|$RECID|" $FNAM-3.sh > $FNAM-4.sh
   rm -f $FNAM-3.sh
   sed -e "s|ARRAYYY|$SUBAR|" $FNAM-4.sh > $FNAM-5.sh
   rm -f $FNAM-4.sh
   sed -e "s|DATASET|$DSET|" $FNAM-5.sh > $FNAM.sh
   rm -f $FNAM-5.sh

   chmod u+x $FNAM.sh
   echo $FNAM.sh

# submit the job
   qsub -l h_cpu=11:49:00 -l os="sl*" -l h_vmem=9000M -l tmpdir_size=5G  -V -j y -o $QLOG -e $QLOG "$FNAM.sh" 
   echo "run script written to $FNAM.sh"
   echo "queue log and error files written to $QLOG"
done

exit

