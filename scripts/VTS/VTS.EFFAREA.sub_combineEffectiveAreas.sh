#!/bin/sh
#
# combine many effective area files into one
#
#
########################################

if [ $# -ne 6 ]
then
   echo 
   echo "VTS.EFFAREA.sub_combineEffectiveAreas.sh <input directory> <search string> <combined file> <array type> <atmosphere> <array IDs>"
   echo
   echo "  example: "
   echo "           ./VTS.EFFAREA.sub_combineEffectiveAreas.sh $VERITAS_DATA_DIR/analysis/EVDv400/EffectiveAreas/130601/ eff-N3-moderate \ "
   echo "                        $VERITAS_USER_DATA_DIR/temp/effArea-d20130411-cut-N3-Point-005CU-Moderate \"V5 V6\" \"21 22\" \"0 1 2 3\" "
   echo
   exit
fi

# run scripts and output is written into this directory
DATE=`date +"%y%m%d"`
QLOG=$VERITAS_USER_LOG_DIR"/queueShellDir/"$DATE/
echo "writing queue log and error files to $QLOG"
LDIR="/dev/null"
if [ ! -d $QLOG ]
then
  mkdir -p $QLOG
  chmod -R g+w $QLOG
fi

# loop over all files/cases
for V in $4
do
   for A in $5
   do
     for I in $6
     do
        echo $V $A $I
	echo

# telescope combinations
        if [ $I -eq "0" ]
	then
	   T="1234"
        elif [ $I -eq "1" ]
        then
	   T="234"
        elif [ $I -eq "2" ]
        then
	   T="134"
        elif [ $I -eq "3" ]
        then
	   T="124"
        elif [ $I -eq "4" ]
        then
	   T="123"
        fi

# input and output names
	DDIR=$1/$2-$A-$V/$2-$A-$V-$I*

	OFIL=`basename $3`
	OFIL="$OFIL-ATM$A-$V-T$T-d20130521"
	ODIR=`dirname $3`

	echo "$DDIR"
	echo "$ODIR"
	echo "$OFIL"

	FNAM="$QLOG/COMB-$2-$A-$V-$I"
	rm -f $FNAM.sh

	sed -e "s|DDDD|$DDIR|" \
	    -e "s|OOOO|$OFIL|" \
	    -e "s|ODOD|$ODIR|" VTS.EFFAREA.qsub_combineEffectiveAreas.sh > $FNAM.sh
	    
	 chmod u+x $FNAM.sh
	 echo $FNAM.sh
# submit job
         qsub -l os="sl*" -l h_cpu=5:29:00 -l h_vmem=6000M -l tmpdir_size=10G -V -o $QLOG -e $QLOG "$FNAM.sh"

	 echo "writing analysis parameter files to $FNAM.sh"
     done
   done
done

