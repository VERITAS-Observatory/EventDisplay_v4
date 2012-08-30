#!/bin/sh
#
# run laser/flasher calibration over a bunch of laser runs 
# reads a simple list of run, gets the corresponding laser run numbers 
# and calculates gains/toff for these runs
# (do this only if gain files don't exist yet)
#
# Revision $Id: evndisp.analyse_laser_runs_from_runlist.sh,v 1.1.2.1.2.2 2010/10/30 03:09:36 gmaier Exp $
#
# Author: Gernot Maier
#

######################################
# location of vbf files of laser/flasher runs 
DDIR=$VERITAS_DATA_DIR"/data/"
# setting the path for binaries
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi
# location of gain/toff file
if [ -z $EVNDISPDATA ]
then
  echo "no EVNDISPDATA env variable defined"
  exit
fi
GDIR="$EVNDISPDATA/calibration/"
######################################

if [ ! -n "$1" ] 
then
   echo "VTS.EVNDISP.analyse_laser_runs_from_runlist.sh <run list> [ntel]"
   echo
   exit
fi

RLIST=$1
NTEL=4
if [ -e "$2" ]
then
  NTEL="$2"
fi


FILES=`cat $RLIST`
for AFIL in $FILES
do

    for ((i=1; i <= $NTEL; i++ ))
    do
      RUN=`$EVNDISPSYS/bin/VTS.getLaserRunFromDB $i $AFIL`

      echo $RUN

      continue

      echo "checking telescope $i laser run $RUN data run $AFIL"

      echo "$GDIR/Tel_"$i"/$RUN.gain.root"
      if [ ! -e "$GDIR/Tel_"$i"/$RUN.gain.root" ] 
      then
	 echo $DDIR 
	 DFIL=`find -L $DDIR -name "$RUN.cvbf"`
	 echo $DFIL
	 if [ -e $DFIL ]
	 then
	    $EVNDISPSYS/scripts/VTS/VTS.EVNDISP.analyse_laser_run $i $DFIL
         else
	    echo "missing laser/flasher file $FIL"
	    echo "please download"
         fi
      else
         echo "...gain and toff file on disk"
      fi
    done
done

exit
