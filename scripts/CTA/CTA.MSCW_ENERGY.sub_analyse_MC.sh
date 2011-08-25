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
   echo "CTA.MSCW_ENERGY.sub_analyse_MC.sh <tablefile> <recid> <subarray> <particle> [LL/GEO] [wildcard]"
   echo
   echo "  <tablefile>     table file name (without .root)"
   echo "  <recid>         reconstruction ID"
   echo "  <subarray>      subarray identifier (A,B,C...)"
   echo "  <particle>      gamma_onSource/gamma_cone10/electron/proton/helium"
   echo
   echo "optional (for a huge amount of MC files):"
   echo "  [LL/GEO]        image reconstruction method (not implemented yet)"
   echo "  [wildcard]     used in the < CTA.MSCW_ENERGY.subParallel_analyse_MC.sh > script"
   exit
fi

#########################################
# input parameters
TABLE=$1
RECID=$2
SUBAR=$3
PART=$4
METH=""
if [ -n "$5" ]
then 
    METH=$5
fi
WC=""
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
# input files
IFIL="$CTA_DATA_DIR/analysis/$SUBAR/$PART/$WC"

# check if input files exist
IFILN=`ls -1 $IFIL*.root | wc -l`
if [ $IFILN -eq 0 ]
then
  echo "No input files in $IFIL"
  echo "exiting..."
  exit
fi
echo "FOUND $IFILN input files in $IFIL"

#########################################
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`
QLOG=$CTA_USER_LOG_DIR/$DATE/
mkdir -p $QLOG

# output directory for shell scripts
SHELLDIR=$CTA_USER_LOG_DIR"/queueShellDir/"
mkdir -p $SHELLDIR

# output file name for mscw_energy
TFIL=$PART$NC"."$SUBAR"_ID"$RECID".mscw"
if [ ${#WC} -gt 0 ]
then 
   TFIL=$PART$NC"."$SUBAR"_ID"$RECID"-$WC.mscw"
fi


# skeleton script
FSCRIPT="CTA.MSCW_ENERGY.qsub_analyse_MC"

FNAM="$SHELLDIR/MSCW.ana-ID$RECID-array$SUBAR"

sed -e "s|TABLEFILE|$TABLE|" $FSCRIPT.sh > $FNAM-1.sh
sed -e "s|IIIIFIL|$IFIL|" $FNAM-1.sh > $FNAM-2.sh
rm -f $FNAM-1.sh
sed -e "s|TTTTFIL|$TFIL|" $FNAM-2.sh > $FNAM-3.sh
rm -f $FNAM-2.sh
sed -e "s|RECONSTRUCTIONID|$RECID|" $FNAM-3.sh > $FNAM-4.sh
rm -f $FNAM-3.sh
sed -e "s|ARRAYYY|$SUBAR|" $FNAM-4.sh > $FNAM.sh
rm -f $FNAM-4.sh

chmod u+x $FNAM.sh
echo $FNAM.sh

# submit the job
qsub -l h_cpu=10:30:00 -l h_vmem=6000M -l tmpdir_size=20G  -V -j y -o $QLOG -e $QLOG "$FNAM.sh" 
#qsub -l h_cpu=04:30:00 -l h_vmem=3500M -l tmpdir_size=5G  -V -j y -o $QLOG -e $QLOG "$FNAM.sh" 
echo "run script written to $FNAM.sh"
echo "queue log and error files written to $QLOG"

exit

