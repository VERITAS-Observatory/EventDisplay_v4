#!/bin/sh
#
# submit effective area analysis
#
# (output need to be combined afterwards)
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
QLOG=$VERITAS_USER_LOG_DIR"/"$DATE/EFFAREA
echo "writing queue log and error files to $QLOG"
LDIR="/dev/null"
# LDIR="$QLOG"
if [ ! -d $QLOG ]
then
  mkdir -p $QLOG
  chmod -R g+w $QLOG
fi
LOGDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/EffectiveAreas/"$DATE/ID$REID-V$ARRAY/
if [ ! -d $LOGDIR ]
then
  mkdir -p $LOGDIR
  chmod -R g+w $LOGDIR
fi
echo "writing log files to $LOGDIR"
ODDIR=$VERITAS_DATA_DIR"/analysis/EVDv400/EffectiveAreas/"$DATE/
if [ ! -d $ODDIR ]
then
  mkdir -p $ODDIR
  chmod -R g+w $ODDIR
fi
echo "writing results to $ODDIR"


############################################################################################
# loop over all zenith, wobble and noise bins
############################################################################################
NZE=${#IZE[@]}
NNOI=${#INOI[@]}
NWOFF=${#WOFF[@]}

# copy cut files to temp directory (one cut file for all sets)
FXIR="$IFIL-$REID"
cp $VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles/$CUTS.dat $LOGDIR/$FXIR-$CUTS.dat

for (( j = 0 ; j < $NNOI; j++ ))
do
   for (( i = 0 ; i < $NZE ; i++ ))
   do
      for (( k = 0; k < $NWOFF; k++ ))
      do
	 let "l = $i + 1"

############################################################################################
# MC files and directory names
############################################################################################
# data file to be analyzed
	 TFIL=gamma_${IZE[$i]}deg_750m_w"${WOFF[$k]}"_ID"$REID"_ana"$ARRAY"_NOISE${INOI[$j]}_1.root
         FFIL=$FDIR"/"$TFIL
	 if [ ! -e $FFIL ]
	 then
	    echo "INPUT FILE NOT FOUND: $FFIL"
         fi
###########################################################################################
###########################################################################################
# directory for parameter and cut files
	 FFIR="$IFIL-$REID-${IZE[$i]}-${WOFF[$k]}-${INOI[$j]}"
# create parameter file
	 rm -f $LOGDIR/$FFIR.dat
	 touch $LOGDIR/$FFIR.dat

echo "
* FILLINGMODE 0
* ENERGYRECONSTRUCTIONMETHOD 1
* ENERGYAXISBINS 60
* AZIMUTHBINS 1
* FILLMONTECARLOHISTOS 0
* ENERGYSPECTRUMINDEX 20 2.0 0.1
* FILLMONTECARLOHISTOS 0
* SHAPECUTINDEX 0
* CUTFILE $LOGDIR/$FXIR-$CUTS.dat
* SIMULATIONFILE_DATA $FFIL" > $LOGDIR/$FFIR.dat

# set parameters in run script
         FNAM="$QLOG/MK-EA.$REID.$DATE.MC"

	 sed -e "s|EFFFILE|$FFIR|" \
	     -e "s|OOOOOOO|$ODDIR|" \
	     -e "s|MSCWFILE|$LOGDIR/$FFIR.dat|" VTS.EFFAREA.qsub_analyse.sh > $FNAM.sh

	 chmod u+x $FNAM.sh
	 echo $FNAM.sh
# submit job
         qsub -l os=sl6 -l h_cpu=11:29:00 -l h_vmem=6000M -l tmpdir_size=10G -V -o $LDIR -e $LDIR "$FNAM.sh"

	 echo "writing run parameter file to $LOGDIR/$FFIR.dat"
	 echo "writing analysis parameter files to $FNAM.sh"

         sleep 0.1
     done
   done
done

exit
