#!/bin/bash
#
# script to check that files where processed correctly
#
# Revision $Id$
#
# Author: Gernot Maier
#
#######################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ] && [ ! -n "$3" ] && [ ! -n "$4" ]
then
   echo "./CTA.EVNDISP.check_convert_and_analyse_MC_VDST.sh <sub array> <list of simtelarray files> <particle> <list of failed jobs>"
   echo
   echo "  <sub array>               sub array from prod1 (e.g. E)"
   echo "                            use ALL for all arrays (A B C D E F G H I J K NA NB)"
   echo "  <particle>                gamma_onSource , gamma_diffuse, proton , electron (helium, ...)"
   echo ""
   echo "  <list of failed jobs>     list of failed jobs" 
   echo ""
   exit
fi

SUBAR=$1
BLIST=$2
PART=$3
FAILED=0
if [ -n "$4" ]
then
   FAILED=$4
fi
FAILED=$FAILED.$PART

#arrays
if [ $SUBAR = "ALL" ]
then
#  VARRAY=( A B C D E F G H I J K NA NB "s4-2-120" "s4-2-85" "s4-1-120" "I-noLST" "I-noSST" "g60" "g85" "g120" "g170" "g240" "s9-2-120" "s9-2-170" )
  VARRAY=( E I J )
#  VARRAY=( "s2-1-75" "s3-1-210" "s3-3-260" "s3-3-346" "s3-4-240" "s4-1-105" "s4-2-170" "s4-3-200" "s4-4-140" "s4-4-150" "s4-5-125" )
else
  VARRAY=( $SUBAR )
fi
NARRAY=${#VARRAY[@]}
for (( N = 0 ; N < $NARRAY; N++ ))
do
   ARRAY=${VARRAY[$N]}
   rm -f $FAILED.$ARRAY.list
   touch $FAILED.$ARRAY.list
done

############################################################################

FILES=`cat $BLIST`

# loop over all files in files loop
for AFIL in $FILES
do

# get run number 
  F1=${AFIL#*run}
#  echo "CHECKING RUN $F1"
  RUN=${F1%___*}

  for (( N = 0 ; N < $NARRAY; N++ ))
  do
      ARRAY=${VARRAY[$N]}

 # check that simtel fil exists
     if [ ! -e $AFIL ]
     then
       echo "NO SIMTEL FILE: $AFIL"
       echo $AFIL >> $FAILED.$ARRAY.list
       continue
     fi

      DDIR="$CTA_USER_DATA_DIR/analysis/$ARRAY/$PART/"
      LDIR="$CTA_USER_LOG_DIR/analysis/$ARRAY/$PART/"

# check evndisp log file
      LFIL=`basename $AFIL .gz`
      if [ -e $LDIR/$LFIL.evndisp.log ]
      then
	 LLINE=`grep "END OF ANALYSIS" $LDIR/$LFIL.evndisp.log`
	 if [ ${#LLINE} -eq 0 ]
	 then
	   echo "INCOMPLETE EVNDISP RUN $LDIR/$LFIL.evndisp.log" 
           echo $AFIL >> $FAILED.$ARRAY.list
           continue
	 fi
	 LLINE=`grep -i "error" $LDIR/$LFIL.evndisp.log`
	 if [ ${#LLINE} -ne 0 ]
	 then
	   echo "ERRORNESS EVNDISP RUN $LDIR/$LFIL.evndisp.log" 
           echo $AFIL >> $FAILED.$ARRAY.list
	   continue
	 fi
      else
         echo "NO EVNDISP LOG FILE: $LDIR/$LFIL.evndisp.log"
         echo $AFIL >> $FAILED.$ARRAY.list
	 continue
      fi

# check that evndisp output file exists
      if [ ! -e $DDIR/$RUN.root ]
      then
        echo "NO EVNDISP FILE: $DDIR/$RUN.root"
        echo $AFIL >> $FAILED.$ARRAY.list
        continue
      fi

# check that evndisp output file size is > 0
      if [ ! -s $DDIR/$RUN.root ]
      then
        echo "ZERO LENGTH EVNDISP FILE: $DDIR/$RUN.root"
        echo $AFIL >> $FAILED.$ARRAY.list
        continue
      fi
  done
   
done

exit
