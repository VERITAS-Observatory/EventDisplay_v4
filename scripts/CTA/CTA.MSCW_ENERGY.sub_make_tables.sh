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

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ]
then
   echo
   echo "CTA.MSCW_ENERGY.sub_make_tables.sh <table file name> <recid> <array ID> <onSource/cone10>"
   echo ""
   echo "  <table file name>  name of the table file (to be written; without .root)"
   echo "  <recid>            reconstruction ID according to EVNDISP.reconstruction.parameter"
   echo "  <array ID>         CTA array ID (e.g. E for array E or HYBRID1 for Hybrid-1 with 61 telescopes)"
   echo "                     use ALL for all arrays (A B C D E F G H I J K NA NB HYBRID1)"
   echo "  <onSource/cone>    calculate tables for on source or different wobble offsets"
   echo
   echo " input data and output directories for tables are fixed in CTA.MSCW_ENERGY.qsub_make_tables.sh"
   exit
fi

#########################################
# input parameters
#########################################
TFIL=$1
RECID=$2
ARRAY=$3
if [ $ARRAY == "ALL" ]
then
  VARRAY=( A B C D E F G H I J K NA NB "s4-2-120" "s4-2-85" "s4-1-120" "I-noLST" "I-noSST" "g60" "g85" "g120" "g170" "g240" "s9-2-120" "s9-2-170" )
#  VARRAY=( "s2-1-75" "s3-1-210" "s3-3-260" "s3-3-346" "s3-4-240" "s4-1-105" "s4-2-170" "s4-3-200" "s4-4-140" "s4-4-150" "s4-5-125" )
else 
  VARRAY=( $ARRAY )
fi
NARRAY=${#VARRAY[@]}
CONE="FALSE"
if [ $4 == "cone10" ] || [ $4 == "cone" ]
then
  CONE="TRUE"
fi


#########################################
# arrays for different wobble offsets
#########################################
DDIR="$CTA_USER_DATA_DIR/analysis/$ARRAY/"
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
   DSUF="gamma_onSource/[3-5]"
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
QLOG=$CTA_USER_LOG_DIR/$DATE/
mkdir -p $QLOG

# output directory for shell scripts
SHELLDIR=$CTA_USER_LOG_DIR"/queueShellDir/"
mkdir -p $SHELLDIR

# skeleton script
   FSCRIPT="CTA.MSCW_ENERGY.qsub_make_tables"

#########################################
#loop over all arrays
#########################################
for (( N = 0 ; N < $NARRAY; N++ ))
do
   ARRAY=${VARRAY[$N]}
   echo "STARTING ARRAY $ARRAY"

# data dir
   DDIR="$CTA_USER_DATA_DIR/analysis/$ARRAY/$DSUF"

# table file
   TAFIL=$TFIL-$ARRAY

#########################################
#loop over wobble offsets
#########################################

   for (( M = 0 ; M < $NOFF; M++ ))
   do
      MINDIST=${OFFMIN[$M]}
      MAXDIST=${OFFMAX[$M]}
      MEANDIST=${OFFMEA[$M]}

# run scripts
      FNAM="$SHELLDIR/MSCW.table-$TAFIL-W$MEANDIST"

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
      sed -e "s|DATADIRECT|$DDIR|" $FNAM-6.sh > $FNAM.sh
      rm -f $FNAM-6.sh

      chmod u+x $FNAM.sh

# submit the job
      qsub -l h_cpu=25:00:00 -l h_vmem=8000M -V -o $QLOG/ -e $QLOG/ "$FNAM.sh"
   done
done

echo "shell scripts are written to $SHELLDIR"
echo "batch output and error files are written to $QLOG"


exit
