#!/bin/bash
#
# script to run over all noise levels and create lookup tables (queue submit)
#
#
# Author: Gernot Maier
#
#

if [ $# -lt 5 ]
then
   echo
   echo "VTS.MSCW_ENERGY.sub_make_tables.sh <table file> <recid> <atmo ID=21/22> <array=V4/V5/V6> <GRISU/CARE>"
   echo
   echo " recid: array rec id according to array_analysis_cuts.txt"
   echo
   echo " <GRISU/CARE>: simulation package used"
   echo
   echo $#
   exit
fi

TFIL=$1
ANAC="1234"
RECID=$2
ARRAY=$4
ATMO=$3
SIM=$5

if [ -e $TFIL ]
then
   echo "error: table file exists, move it or delete it"
   exit
fi

FSCRIPT="VTS.MSCW_ENERGY.qsub_make_tables"

#################################################################
# zenith angle bins
IZE=( 00 20 30 35 40 45 50 55 60 65 ) 
NZE=${#IZE[@]}
#################################################################
# wobble off bins
WOFF=( 0.00 0.25 0.5 0.75 1.00 1.25 1.50 1.75 2.00 )
WOFF=( 0.5 )
NWOF=${#WOFF[@]}
#################################################################
# noise levels
##################
# GRISU  Simulations
if [ $SIM = "GRISU" ]
then
    INOI=( 075 100 150 200 250 325  425  550  750 1000 )
# mean ped var
# (short summation window)
# determined from mean pedvar of simulations
# (cannot be determined on the fly due to floating point uncertainty)
# from sumwindow=6
    if [ $ARRAY = "V5" ] || [ $ARRAY = "V4" ]
    then
       TNOI=( 336 382 457 524 579 659 749 850 986 1138 )
    else
       TNOI=( 331 384 469 542 600 684 773 875 1002 1178 )
    fi
# from sumwindow=7
# TNOI=( 375 430 515 585 650 740 840 950 1105 1280 )
# from sumwindow=12
# TNOI=( 535 610 730 840 935 1060 1210 1370 1595 1840 )
##################
# CARE Simulations
else
# from sumwindow=6
    INOI=(  50  80 120 170 230 290  370  450 )
    TNOI=( 349 410 607 683 800 888 1001 1108 )
fi
NNOI=${#INOI[@]}
#################################################################


DATE=`date +"%y%m%d"`

QLOG=$VERITAS_USER_LOG_DIR"/"$DATE/MSCW.MAKETABLES/
if [ ! -d $QLOG ]
then
  mkdir -p $QLOG
fi
echo "SHELLS AND LOG FILES: $QLOG"
LOGDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/Tables/"$DATE/
if [ ! -d $LOGDIR ]
then
  mkdir -p $LOGDIR
fi
ODDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/Tables/"$DATE/
if [ ! -d $ODDIR ]
then
  mkdir -p $ODDIR
fi


##############################################
# loop over all noise level
for (( i = 0 ; i < $NNOI; i++ ))
do
##############################################
# loop over all zenith angle bins
   N=0
   while [ $N -lt $NZE ]
   do
##############################################
# loop over all wobble offsets
       echo "ZE BIN $N $NZE ${IZE[$N]}"
       W=0
       while [ $W -lt $NWOF ]
       do
	 echo "   WOFF BIN $W $NWOF ${WOFF[$W]}"
	 echo "   LOG DIR $LOGDIR"
	 echo "   DATA DIR WITH TABLES $ODDIR"

	 FNAM="$QLOG/$ARRAY-MK-TBL.$DATE.MC-$ATMO-$ANAC-$RECID-${INOI[$i]}-${IZE[$N]}-${WOFF[$W]}-$ARRAY"
	 rm -f $FNAM.sh

	 sed -e "s|TABLEFILE|$TFIL|" \
	     -e   "s|TELESCOPES|$ANAC|" \
	     -e "s|RECONSTRUCTIONID|$RECID|" \
	     -e "s|NOISEI|${INOI[$i]}|" \
	     -e "s|AAAAA|$ARRAY|" \
	     -e "s|BBBBBBB|$ATMO|" \
	     -e "s|NOISET|${TNOI[$i]}|" \
	     -e "s|THEDATE|$DATE|" \
	     -e "s|CCCCCC|$LOGDIR|" \
	     -e "s|DDDDDD|$ODDIR|" \
	     -e "s|ZENITH|${IZE[$N]}|" \
             -e "s|SIMS|$SIM|" \
	     -e "s|WOBBLEOFFSET|${WOFF[$W]}|" $FSCRIPT.sh > $FNAM.sh

	 chmod u+x $FNAM.sh

# submit job
	qsub -V -l os=sl6 -l h_cpu=3:29:00 -l h_vmem=4000M -l tmpdir_size=1G -o $QLOG/ -e $QLOG/ "$FNAM.sh"

        let "W = $W + 1"
     done
     let "N = $N + 1"
  done
done

exit
