#!/bin/bash
#
# script to run over all noise levels and create lookup tables (queue submit)
#
# Revision $Id: make_tables.sh,v 1.1.2.2.4.2.12.2.4.1.2.2.2.1.6.2.2.3.2.4.2.2.6.1 2011/03/22 08:27:33 gmaier Exp $
#
#
# Author: Gernot Maier
#
#

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] || [ ! -n "$4" ] || [ ! -n "$5" ]  || [ ! -n "$6" ] || [ ! -n "$7" ]
then
   echo "VTS.MSCW_ENERGY.sub_make_tables.sh <table file> <ana combination> <recid> <summation window=07/12> <array=V4/V5> <recmethod=GEO/LL> <atmo ID=06/20/21/22>"
   echo
   echo "ana combinations are: 12, 123, 1234, 24, etc."
   echo "recid: array rec id according to array_analysis_cuts.txt"
   echo
   exit
fi

TFIL=$1
ANAC=$2
RECID=$3
SUMW=$4
ARRAY=$5
METH=$6
ATMO=$7

if [ -e $TFIL ]
then
   echo "error: table file exists, move it or delete it"
   exit
fi

FSCRIPT="VTS.MSCW_ENERGY.qsub_make_tables"

#################################################################
# noise levels
INOI=( 075 100 150 200 250  325  425  550  750 1000 )
#################################################################
if [ $ARRAY = "V4" ]
then
  if [ $SUMW = "12" ]
  then
    TNOI=( 535 610 730 840 935 1060 1210 1370 1595 1840 )
  fi
  if [ $SUMW = "07" ]
  then
    TNOI=( 375 430 515 585 650 740 840 950 1105 1280 )
  fi
fi
#################################################################
# V5
if [ $ARRAY = "V5" ]
then
  if [ $SUMW = "12" ]
  then
    TNOI=( 535 610 730 840 935 1060 1210 1370 1595 1840 )
  fi
  if [ $SUMW = "07" ]
  then
    TNOI=( 375 430 515 585 650 740 840 950 1105 1280 )
  fi
fi

NNOI=${#INOI[@]}

DATE=`date +"%y%m%d"`

QLOG=$VERITAS_USER_LOG_DIR"/queueShellDir/"$DATE/
if [ ! -d $QLOG ]
then
  mkdir -p $QLOG
fi
echo "SHELLS AND LOG FILES: $QLOG"
LOGDIR=$VERITAS_USER_LOG_DIR"/analysis/EVDv400/Tables/"$DATE/
if [ ! -d $LOGDIR ]
then
  mkdir -p $LOGDIR
fi
ODDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/Tables/"$DATE/
if [ ! -d $ODDIR ]
then
  mkdir -p $ODDIR
fi


for (( i = 0 ; i < $NNOI; i++ ))
do
      #FNAM="$QLOG/MK-TBL.$DATE.MC"

      FNAM="$QLOG/MK-TBL.$DATE.MC-$ATMO-$METH-$ANAC-$RECID-${INOI[$i]}-$SUMW-$ARRAY"

      sed -e "s|TABLEFILE|$TFIL|" $FSCRIPT.sh > $FNAM-2.sh
      sed -e "s|TELESCOPES|$ANAC|" $FNAM-2.sh > $FNAM-3.sh
      rm -f $FNAM-2.sh
      sed -e "s|RECONSTRUCTIONID|$RECID|" $FNAM-3.sh > $FNAM-4.sh
      rm -f $FNAM-3.sh
      sed -e "s|NOISEI|${INOI[$i]}|" $FNAM-4.sh > $FNAM-5.sh
      rm -f $FNAM-4.sh
      sed -e "s|WWWWW|$SUMW|" $FNAM-5.sh > $FNAM-6.sh
      rm -f $FNAM-5.sh
      sed -e "s|AAAAA|$ARRAY|" $FNAM-6.sh > $FNAM-7.sh 
      rm -f $FNAM-6.sh
      sed -e "s|BBBBBBB|$ATMO|" $FNAM-7.sh > $FNAM-8.sh 
      rm -f $FNAM-7.sh
      sed -e "s|RECMETH|$METH|" $FNAM-8.sh > $FNAM-9.sh 
      rm -f $FNAM-8.sh
      sed -e "s|NOISET|${TNOI[$i]}|" $FNAM-9.sh > $FNAM-10.sh
      rm -f $FNAM-9.sh
      sed -e "s|THEDATE|$DATE|" $FNAM-10.sh > $FNAM-11.sh
      rm -f $FNAM-10.sh
      sed -e "s|CCCCCC|$LOGDIR|" $FNAM-11.sh > $FNAM-12.sh
      rm -f $FNAM-11.sh
      sed -e "s|DDDDDD|$ODDIR|" $FNAM-12.sh > $FNAM.sh
      rm -f $FNAM-12.sh

      chmod u+x $FNAM.sh

# submit job
     #qsub -V -l h_cpu=45:29:00 -l h_vmem=8000M -l tmpdir_size=10G -o $QLOG/run/ -e $QLOG/run/ "$FNAM.sh"
     qsub -V -l os="sl*" -l h_cpu=06:29:00 -l h_vmem=6000M -l tmpdir_size=100G -o $QLOG/ -e $QLOG/ "$FNAM.sh"
done

exit
