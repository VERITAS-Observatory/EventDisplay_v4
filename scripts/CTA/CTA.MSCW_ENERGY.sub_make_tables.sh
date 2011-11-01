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

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ]
then
   echo
   echo "CTA.MSCW_ENERGY.sub_make_tables.sh <table file name> <recid> <array ID> [min distance to camera center] [max distance to camera centre] [mean distance to camera centre]"
   echo ""
   echo "  <table file name>  name of the table file (to be written; without .root)"
   echo "  <recid>            reconstruction ID according to EVNDISP.reconstruction.parameter"
   echo "  <array ID>         CTA array ID (e.g. E for array E)"
   echo "                     use ALL for all arrays (A B C D E F G H I J K NA NB)"
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
  VARRAY=( A B C D E F G H I J K NA NB )
else 
  VARRAY=( $ARRAY )
fi
NARRAY=${#VARRAY[@]}

MINDIST="-1.e10"
if [ -n "$4" ]
then
   MINDIST=$4
fi
MAXDIST="1.e10"
if [ -n "$5" ]
then
   MAXDIST=$5
fi
MEANDIST="0.0"
if [ -n "$6" ]
then
   MEANDIST=$6
fi

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

#loop over all arrays
for (( N = 0 ; N < $NARRAY; N++ ))
do
   ARRAY=${VARRAY[$N]}
   echo "STARTING ARRAY $ARRAY"

# table file
   TAFIL=$TFIL-$ARRAY

# skeleton script
   FSCRIPT="CTA.MSCW_ENERGY.qsub_make_tables"

   FNAM="$SHELLDIR/MSCW.table-$TAFIL"

   sed -e "s|TABLEFILE|$TAFIL|" $FSCRIPT.sh > $FNAM-1.sh
   sed -e "s|RECONSTRUCTIONID|$RECID|" $FNAM-1.sh > $FNAM-2.sh
   rm -f $FNAM-1.sh
   sed -e "s|WOMIIIIIN|$MINDIST|" $FNAM-2.sh > $FNAM-3.sh
   rm -f $FNAM-2.sh
   sed -e "s|WOMEEEEAN|$MEANDIST|" $FNAM-3.sh > $FNAM-4.sh
   rm -f $FNAM-3.sh
   sed -e "s|ARRRRRRR|$ARRAY|" $FNAM-4.sh > $FNAM-5.sh
   rm -f $FNAM-4.sh
   sed -e "s|WOMAXXXXX|$MAXDIST|" $FNAM-5.sh > $FNAM.sh
   rm -f $FNAM-5.sh

   chmod u+x $FNAM.sh

# submit the job
   qsub -l h_cpu=12:00:00 -l h_vmem=8000M -V -o $QLOG/ -e $QLOG/ "$FNAM.sh"
done

echo "batch output and error files are written to $QLOG"


exit
