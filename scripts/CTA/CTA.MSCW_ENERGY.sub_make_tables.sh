#!/bin/sh
#
# make tables for CTA
#
# Revision $Id: make_tables_CTA.sh,v 1.1.2.1.2.1 2011/01/03 08:23:40 gmaier Exp $
#
#
# Author: Gernot Maier
#
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ]  && [ ! -n "$5" ]
then
   echo
   echo "CTA.MSCW_ENERGY.sub_make_tables.sh <table file name> <recid> <subarray list> <onSource/cone10> <data set>"
   echo ""
   echo "  <table file name>  name of the table file (to be written; without .root)"
   echo "  <recid>            reconstruction ID according to EVNDISP.reconstruction.parameter"
   echo "  <subarray list>    text file with list of subarray IDs"
   echo "  <onSource/cone10>    calculate tables for on source or different wobble offsets"
   echo "  <data set>         e.g. cta-ultra3, ISDC3700m, ...  "
   echo
   echo " input data and output directories for tables are fixed in CTA.MSCW_ENERGY.qsub_make_tables.sh"
   exit
fi

#########################################
# input parameters
#########################################
TFIL=$1
RECID=$2
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $3`
CONE="FALSE"
if [ $4 == "cone10" ] || [ $4 == "cone" ]
then
  CONE="TRUE"
fi
DSET=$5


#########################################
# arrays for different wobble offsets
#########################################
if [ $CONE == "TRUE" ]
then
   OFFMIN=( 0.0 1.0 2.0 3.00 3.50 4.00 4.50 5.00 5.50 )
   OFFMAX=( 1.0 2.0 3.0 3.50 4.00 4.50 5.00 5.50 6.00 )
   OFFMEA=( 0.5 1.5 2.5 3.25 3.75 4.25 4.75 5.25 5.75 )
   DSUF="gamma_cone10/1"
else
   OFFMIN=( "-1.e10" )
   OFFMAX=( "1.e10" )
   OFFMEA=( 0.0 )
   DSUF="gamma_onSource/[1-9]"
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
SHELLDIR=$CTA_USER_LOG_DIR"/queueShellDir/"
mkdir -p $SHELLDIR

# skeleton script
   FSCRIPT="CTA.MSCW_ENERGY.qsub_make_tables"

#########################################
#loop over all arrays
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
      FNAM="$SHELLDIR/MSCW.table-$TAFIL-W$MEANDIST-$ARRAY"

      sed -e "s|TABLEFILE|$TAFIL|" $FSCRIPT.sh > $FNAM-1.sh
      sed -e "s|RECONSTRUCTIONID|$RECID|" $FNAM-1.sh > $FNAM-2.sh
      rm -f $FNAM-1.sh
      sed -e "s|WOMIIIIIN|$MINDIST|" $FNAM-2.sh > $FNAM-3.sh
      rm -f $FNAM-2.sh
      sed -e "s|WOMEEEEAN|$MEANDIST|" $FNAM-3.sh > $FNAM-4.sh
      rm -f $FNAM-3.sh
      sed -e "s|ARRRRRRR|$ARRAY|" $FNAM-4.sh > $FNAM-5.sh
      rm -f $FNAM-4.sh
      sed -e "s|WOMAXXXXX|$MAXDIST|" $FNAM-5.sh > $FNAM-6.sh
      rm -f $FNAM-5.sh
      sed -e "s|DATADIRECT|$DDIR|" $FNAM-6.sh > $FNAM-7.sh
      rm -f $FNAM-6.sh
      sed -e "s|DATASET|$DSET|" $FNAM-7.sh > $FNAM.sh
      rm -f $FNAM-7.sh

      chmod u+x $FNAM.sh

# submit the job
      qsub -l os="sl*" -js 20 -l h_cpu=11:00:00 -l h_vmem=8000M -V -o $QLOG/ -e $QLOG/ "$FNAM.sh"
   done
done

echo "shell scripts are written to $SHELLDIR"
echo "batch output and error files are written to $QLOG"


exit
