#!/bin/sh
#
# script to analyse data files with lookup tables
#
# Author: Gernot Maier
#
############################################################################################
#
############################################################################################

if [ ! -n "$1" ] && [ ! -n "$2" ]  && [ ! -n "$3" ]  && [ ! -n "$4" ] && [ ! -n "$5" ] && [ ! -n "$6" ]
then
   echo
   echo "VTS.EFFAREA.sub_analyse.sh <cutfile (without .dat)> <output file name (without suffix)> <reconstruction ID> <array config (e.g. 123)> <data directory>" 
   echo
   echo "   Note: zenith, wobble offset, and noise range hardwired"
   echo "         data file with MC parameters is expected to be in this directory: <data directory>G"
   echo
   exit
fi

CUTS=$1
IFIL=$2
REID=$3
ARRAY=$4
FDIR="$5"
NAME=$6
############################################################################################
IZE=( 00 20 30 35 40 45 50 55 60 65 )
INOI=( 075 100 150 200 250 325 425 550 750 1000 )
WOFF=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
############################################################################################
# run scripts and output is written into this directory
DATE=`date +"%y%m%d"`
QLOG=$VERITAS_USER_LOG_DIR"/queueShellDir/"$DATE/
if [ ! -d $QLOG ]
then
  mkdir -p $QLOG
  chmod -R g+w $QLOG
fi
LOGDIR=$VERITAS_USER_LOG_DIR"/analysis/EVDv400/EffectiveAreas/"$DATE/
if [ ! -d $LOGDIR ]
then
  mkdir -p $LOGDIR
  chmod -R g+w $LOGDIR
fi
ODDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/EffectiveAreas/"$DATE/
if [ ! -d $ODDIR ]
then
  mkdir -p $ODDIR
  chmod -R g+w $ODDIR
fi


############################################################################################
# loop over all zenith, wobble and noise bins
############################################################################################
NZE=${#IZE[@]}
NNOI=${#INOI[@]}
NWOFF=${#WOFF[@]}

for (( j = 0 ; j < $NNOI; j++ ))
do
   for (( i = 0 ; i < $NZE ; i++ ))
   do
      for (( k = 0; k < $NWOFF; k++ ))
      do
	 let "l = $i + 1"

############################################################################################
# hard wired file and directory names
############################################################################################
# data file to be analyzed
	 TFIL=gamma_${IZE[$i]}deg_750m_w"${WOFF[$k]}"_ID"$REID"_ana"$ARRAY"_NOISE${INOI[$j]}_1.root
         FFIL=$FDIR"/"$TFIL
	 if [ ! -e $FFIL ]
	 then
	    echo "INPUT FILE NOT FOUND: $FFIL"
# ttt	    exit
         fi
# data file with MC data and parameters
	 XFIL="$FDIR"/gamma_${IZE[$i]}deg_750m_w"${WOFF[$k]}"_ID"$REID"_ana1234_NOISE250_1.root
	 if [ ! -e $XFIL ]
	 then
	    echo "INPUT FILE (MC) NOT FOUND: "$FDIR"/$XFIL.root"
# ttt	    exit
         fi
###########################################################################################
###########################################################################################
# directory for parameter and cut files
	 FFIR="$IFIL-$REID-${IZE[$i]}-${WOFF[$k]}-${INOI[$j]}"
# cp cut files to temp directory
         cp $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/$CUTS.dat $LOGDIR/$FFIR-$CUTS.dat
# create parameter file
	 rm -f $LOGDIR/$FFIR.dat
	 touch $LOGDIR/$FFIR.dat

         echo "* FILLINGMODE 0" >> $LOGDIR/$FFIR.dat
	 echo "* ENERGYRECONSTRUCTIONMETHOD 1" >> $LOGDIR/$FFIR.dat
	 echo "* ENERGYAXISBINS 60" >> $LOGDIR/$FFIR.dat
         echo "* AZIMUTHBINS 1" >> $LOGDIR/$FFIR.dat
	 echo "* FILLMONTECARLOHISTOS 0" >> $LOGDIR/$FFIR.dat
	 echo "* ENERGYSPECTRUMINDEX 20 2.0 0.1" >> $LOGDIR/$FFIR.dat
	 echo "* FILLMONTECARLOHISTOS 0" >> $LOGDIR/$FFIR.dat
	 echo "* SHAPECUTINDEX 0" >> $LOGDIR/$FFIR.dat
	 echo "* CUTFILE $LOGDIR/$FFIR-$CUTS.dat" >> $LOGDIR/$FFIR.dat
	 echo >> $LOGDIR/$FFIR.dat
	 echo "* SIMULATIONFILE_DATA $FFIL" >> $LOGDIR/$FFIR.dat

# set parameters in run script
         FNAM="$QLOG/MK-EA.$REID.$DATE.MC"

	 sed -e "s|EFFFILE|$FFIR|" VTS.EFFAREA.qsub_analyse.sh > $FNAM-3.sh
	 sed -e "s|OOOOOOO|$ODDIR|" $FNAM-3.sh > $FNAM-4.sh
	 rm -f $FNAM-3.sh
	 sed -e "s|MSCWFILE|$LOGDIR/$FFIR.dat|" $FNAM-4.sh > $FNAM.sh
	 rm -f $FNAM-4.sh

	 chmod u+x $FNAM.sh
	 echo $FNAM.sh
	 echo "DATA DIR: $ODDIR"
	 echo "LOG DIR: $LOGDIR"
# submit job
         qsub -l os="sl*" -l h_cpu=11:29:00 -l h_vmem=6000M -l tmpdir_size=10G -V -o $QLOG/ -e $QLOG/ "$FNAM.sh"

	 echo "writing queue log and error files to $QLOG"
	 echo "writing analysis parameter files to $FNAM.sh"
	 echo "writing results to $ODDIR"
	 echo "writing log files to $LOGDIR"

         sleep 0.1
     done
   done
done

exit
