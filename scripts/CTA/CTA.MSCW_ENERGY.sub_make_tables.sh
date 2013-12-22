#!/bin/sh
#
# make tables for CTA
#
#
# Author: Gernot Maier
#
#

if [ $# -lt 5 ]
then
   echo
   echo "CTA.MSCW_ENERGY.sub_make_tables.sh <table file name> <recid> <subarray list> <onSource/cone> <data set> [qsub options]"
   echo ""
   echo "  <table file name>  name of the table file (to be written; without .root)"
   echo "  <recid>            reconstruction ID according to EVNDISP.reconstruction.parameter"
   echo "  <subarray list>    text file with list of subarray IDs"
   echo "  <onSource/cone>    calculate tables for on source or different wobble offsets"
   echo "  <data set>         e.g. cta-ultra3, ISDC3700m, ...  "
   echo
   echo " input data and output directories for tables are fixed in CTA.MSCW_ENERGY.qsub_make_tables.sh"
   echo
   echo " tables create for different wobble offsets can be combined with CTA.MSCW_ENERGY.combine_tables.sh"
   echo
   exit
fi

#########################################
# input parameters
#########################################
TFIL=$1
RECID=$2
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $3`
CONE="FALSE"
if [ $4 == "cone" ]
then
  CONE="TRUE"
fi
DSET=$5
if [ -n $6 ]
then
   QSUBOPT="$6"
fi
QSUBOPT=${QSUBOPT//_X_/ } 
QSUBOPT=${QSUBOPT//_M_/-} 

#########################################
# arrays for different wobble offsets
#########################################
if [ $CONE == "TRUE" ]
then
#   OFFMIN=( 0.0 1.0 2.0 3.00 3.50 4.00 4.50 5.00 5.50 )
#   OFFMAX=( 1.0 2.0 3.0 3.50 4.00 4.50 5.00 5.50 6.00 )
#   OFFMEA=( 0.5 1.5 2.5 3.25 3.75 4.25 4.75 5.25 5.75 )
   OFFMIN=( "-1.e10" )
   OFFMAX=( "1.e10" )
   OFFMEA=( 0.0 )
   CTAOFF="-CTAoffAxisBins"
   DSUF="gamma_cone/[1-9]"
else
   OFFMIN=( "-1.e10" )
   OFFMAX=( "1.e10" )
   OFFMEA=( 0.0 )
   CTAOFF=""
   DSUF="gamma_onSource/[1-9]"
   TFIL="$TFIL-onAxis"
fi
NOFF=${#OFFMIN[@]}

#########################################
# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS environmental variable defined"
    exit
fi
# checking for table name
if [ -e $TFIL.root ]
then
   echo "error: table file exists, move it or delete it"
   exit
fi

#########################################
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`
QLOG=$CTA_USER_LOG_DIR/$DATE/MAKETABLES/
mkdir -p $QLOG

# output directory for shell scripts
SHELLDIR=$CTA_USER_LOG_DIR/$DATE/MAKETABLES/
mkdir -p $SHELLDIR

# skeleton script
   FSCRIPT="CTA.MSCW_ENERGY.qsub_make_tables"

#########################################
# loop over all arrays
#########################################
for ARRAY in $VARRAY
do
   echo "STARTING ARRAY $ARRAY"

# data dir
   DDIR="$CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$ARRAY/$DSUF"

# table file
   TAFIL=$TFIL

#########################################
#loop over wobble offsets
#########################################

   for (( M = 0 ; M < $NOFF; M++ ))
   do
      MINDIST=${OFFMIN[$M]}
      MAXDIST=${OFFMAX[$M]}
      MEANDIST=${OFFMEA[$M]}

# run scripts
      FNAM="$SHELLDIR/EMSCW.table-$TAFIL-W$MEANDIST-$ARRAY"
      cp $FSCRIPT.sh $FNAM.sh
      cp $FSCRIPT.sh $FNAM.sh

      sed -i -e "s|TABLEFILE|$TAFIL|" \
             -e "s|RECONSTRUCTIONID|$RECID|" \
             -e "s|WOMIIIIIN|$MINDIST|" \
             -e "s|WOMEEEEAN|$MEANDIST|" \
             -e "s|ARRRRRRR|$ARRAY|" \
             -e "s|WOMAXXXXX|$MAXDIST|" \
             -e "s|DATADIRECT|$DDIR|" \
             -e "s|DATASET|$DSET|" \
	     -e "s|CTAOFF|$CTAOFF|" $FNAM.sh

      chmod u+x $FNAM.sh
      echo "shell script " $FNAM.sh

# submit the job
      qsub $QSUBOPT -l os=sl6 -l h_cpu=47:45:00 -l h_vmem=16000M -V -o $QLOG/ -e $QLOG/ "$FNAM.sh"
   done
done

echo "shell scripts are written to $SHELLDIR"
echo "batch output and error files are written to $QLOG"


exit
