#!/bin/sh
#
# script to combine several table file into one
#
# Revision $Id: combineTableFiles.sh,v 1.1.2.1 2011/04/18 07:48:45 gmaier Exp $
#
#
# Author: Gernot Maier
#
#

if [ ! -n "$1" ] && [ ! -n "$2" ] 
then
   echo "VTS.MSCW_ENERGY.sub_combine_table.sh <file with list of tables> <output file name (no .root)>"
   echo
   exit
fi

DATE=`date +"%y%m%d"`

QLOG=$VERITAS_USER_LOG_DIR"/queueShellDir/"$DATE/
if [ ! -d $QLOG ]
then
  mkdir -p $QLOG
  chmod -R g+w $QLOG
fi
LOGDIR=$VERITAS_USER_LOG_DIR"/analysis/Tables/"$DATE/
if [ ! -d $LOGDIR ]
then
  mkdir -p $LOGDIR
  chmod -R g+w $LOGDIR
fi
ODDIR=$VERITAS_DATA_DIR"/analysis/Tables/"$DATE/
if [ ! -d $ODDIR ]
then
  mkdir -p $ODDIR
  chmod -R g+w $ODDIR
fi

FILLIST=$1
OFIL=$ODDIR/$2

if [ -e $OFIL ]
then
   echo "error: table file exists, move it or delete it"
   exit
fi

FDIR=`pwd`
FSCRIPT="VTS.MSCW_ENERGY.qsub_combine_tables"
FNAM="$QLOG/CMB-TBL.$DATE.MC"

sed -e "s|LLIISSTT|$FILLIST|" $FSCRIPT.sh > $FNAM-1.sh
sed -e "s|OOFFIILLEE|$OFIL|" $FNAM-1.sh > $FNAM.sh
rm -f  $FNAM-1.sh

chmod u+x $FNAM.sh

# submit the job
qsub -l h_cpu=20:29:00 -l h_vmem=8000M -V -o $QLOG/ -e $QLOG/ "$FNAM.sh"


exit
