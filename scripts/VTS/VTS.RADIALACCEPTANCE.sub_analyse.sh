#!/bin/sh
#
# calculate radial acceptances
#
#
########################################

if [ $# -ne 5 ]
then
   echo 
   echo "VTS.RADIALACCEPTANCE.sub_analyse.sh <input directory> <input file list> <acceptance file> <array type> <array IDs>"
   echo
   echo "  example: "
   echo "   ./VTS.RADIALACCEPTANCE.sub_analyse.sh \$VERITAS_USER_DATA/analysis/Results/RadialAcceptances/EVD410 \$VERITAS_USER_DATA/analysis/Results/RadialAcceptances/runlist \$VERITAS_USER_DATA/analysis/Results/RadialAcceptances/rad \"V6\" \"0\" "
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
   for C in N2-Point-005CU-Soft N3-Point-005CU-Hard N3-Point-005CU-Moderate N3-Point-005CU-Soft N2-Point-005CU-SuperSoft
   do
     for I in $5
     do
        echo $V $C $I
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

# cut files
        CUT=ANASUM.GammaHadron.d20130411-cut-$C.dat

# run list
        LLIST=$2-$V.dat

# input and output names
        DDIR=$1/RecID$I/
	OFIL=$3-d20130411-cut-$C-$V-T$T

	echo "$DDIR"
	echo "$OFIL"

	FNAM="$QLOG/RADA-$C-$V-$I"
	rm -f $FNAM.sh

	sed -e "s|DDDD|$DDIR|" \
	    -e "s|OOOO|$OFIL|" \
	    -e "s|CCCC|$CUT|" \
	    -e "s|LLLL|$LLIST|" VTS.RADIALACCEPTANCE.qsub_analyse.sh > $FNAM.sh
	    
	 chmod u+x $FNAM.sh
# submit job
         qsub -js 200 -l os="sl*" -l h_cpu=0:29:00 -l h_vmem=6000M -l tmpdir_size=10G -V -o $QLOG -e $QLOG "$FNAM.sh"

	 echo "writing analysis parameter files to $FNAM.sh"
     done
   done
done

