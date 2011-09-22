#!/bin/sh
#
# make effective areas for CTA
#
#
# Revision $Id$
#
# Author: Gernot Maier
#


if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ] || [ ! -n "$5" ]
then
   echo
   echo "CTA.EFFAREA.sub_analyse.sh <array> <recid> <particle> <input> <cutfile template> <outputdirectory> [erec method]"
   echo "================================================================================"
   echo
   echo "make effective areas for CTA"
   echo 
   echo "(note: shape cuts hardwired)"
   echo
   echo "<array>"
   echo "     subarray for analysis (e.g. E)"
   echo
   echo "<recid>"
   echo "     reconstruction ID from array reconstruction"
   echo
   echo "<particle>"
   echo "     gamma_onSource / gamma_cone10 / electron / proton / proton_onSource / helium "
   echo
   echo "<input>"
   echo "     msc      use mscw files as input (slow, but necessary at least once)"
   echo "     eff      use effective area files (note: hardwired file location)"
   echo 
   echo "<cutfile template>"
   echo "     template for gamma/hadron cut file"
   echo
   echo "<outputdirectory>"
   echo "     directory with all result and log files"
   echo
   echo "[erec method]"
   echo "     energy reconstruction method (default=0)"
   echo
   exit
fi

echo
echo "make effective areas for CTA: create run scripts"
echo "------------------------------------------------"
echo

######################################################################
# input variables
######################################################################
ARRAY=$1
RECID=$2
PART=$3
INPU=$4
CFIL=$5
ODIR=$6
EREC=0
if [ -n "$7" ]
then
  EREC="$7"
fi

# check particle type
if [ $PART != "gamma_onSource" ] && [ $PART != "gamma_cone10" ] && [ $PART != "proton" ] && [ $PART != "electron" ] && [ $PART != "helium" ] && [ $PART != "proton_onSource" ]
then
   echo "unknown particle type: " $PART
   exit
fi

######################################################################
# qsub script
######################################################################
FSCRIPT="CTA.EFFAREA.qsub_analyse"

######################################################################
# directories
######################################################################
DATE=`date +"%y%m%d"`
echo "directory for qsub output/error files"
QLOGDIR=$CTA_USER_LOG_DIR"/queueLogDir"
echo $QLOGDIR
mkdir -p $QLOGDIR
echo "directory for qsub shell scripts"
QSHELLDIR=$CTA_USER_LOG_DIR"/queueShellDir"
echo $QSHELLDIR
mkdir -p $QSHELLDIR
echo "data (input) directory"
DDIR=$CTA_DATA_DIR/analysis/$ARRAY/Analysis/
echo $DDIR
mkdir -p $DDIR
echo "output log directory"
FDIR=$CTA_USER_LOG_DIR"/queueEffArea/$DATE/"
echo $FDIR
mkdir -p $FDIR
echo "output data directory"
#ODIR=$CTA_USER_DATA_DIR"/analysis/EffectiveArea/$ARRAY/$DATE/"
echo $ODIR
mkdir -p $ODIR

######################################################################
# input files
######################################################################
# data directory
# on source gamma rays
if [ $PART = "gamma_onSource" ]
then
   MSCFILE=$DDIR/gamma_onSource."$ARRAY"_ID"$RECID".mscw.root
   EFFFILE=$DDIR/EffectiveAreas/
   OFIL=gamma_onSource."$ARRAY"_ID"$RECID".eff
   OFFMIN=( 0. )
   OFFMAX=( 100000. )
# NOTE: this is theta2
   THETA2MIN=( -1. )
#   THETA2MAX=( 0.008 )
#   THETA2MAX=( 0.04 )
# using TMVA
   THETA2MAX=( -1. )
   ISOTROPY="0"
   AZBINS="0"
   TELTYPECUTS="1"
   DIRECTIONCUT="2"
fi
# isotropic gamma-rays: analyse in rings in camera distance
if [ $PART = "gamma_cone10" ]
then
   MSCFILE=$DDIR/gamma_cone10*."$ARRAY"_ID"$RECID".mscw.root
   EFFFILE=$DDIR/EffectiveAreas/
   OFIL=gamma_cone10.v2."$ARRAY"_ID"$RECID".eff
   OFFMIN=( 0. 1. 2. 3.0 3.5 4.0 4.5 5.0 5.5 )
   OFFMAX=( 1. 2. 3. 3.5 4.0 4.5 5.0 5.5 6.0 )
# NOTE: this is theta2
   THETA2MIN=( -1. )
   THETA2MAX=( 0.008 )
   ISOTROPY="0"
   AZBINS="0"
   TELTYPECUTS="1"
   DIRECTIONCUT="2"
fi
if [ $PART = "electron" ]
then
   MSCFILE=$DDIR/electron*."$ARRAY"_ID"$RECID".mscw.root
   EFFFILE=$DDIR/EffectiveAreas/
   OFIL=electron."$ARRAY"_ID"$RECID".eff
   OFFMIN=( 0. )
   OFFMAX=( 100000. )
# NOTE: this is theta and not theta2
   THETA2MIN=( 0. 1. 2. 3.0 3.5 4.0 4.5 5.0 5.5 )
   THETA2MAX=( 1. 2. 3. 3.5 4.0 4.5 5.0 5.5 6.0 )
   ISOTROPY="1"
   AZBINS="0"
   TELTYPECUTS="1"
   DIRECTIONCUT="0"
fi
if [ $PART = "proton" ] || [ $PART = "proton_onSource" ]
then
   MSCFILE=$DDIR/proton*."$ARRAY"_ID"$RECID"-*.mscw.root
   EFFFILE=$DDIR/EffectiveAreas/
   OFIL=proton.v2."$ARRAY"_ID"$RECID".eff
   OFFMIN=( 0. )
   OFFMAX=( 100000. )
# NOTE: this is theta and not theta2
   if [ $PART = "proton" ] 
   then
      THETA2MIN=( 0. 1. 2. 3.0 3.5 4.0 4.5 5.0 5.5 )
      THETA2MAX=( 1. 2. 3. 3.5 4.0 4.5 5.0 5.5 6.0 )
   else
      THETA2MIN=( 0. )
      THETA2MAX=( 1. )
   fi
   ISOTROPY="1"
   AZBINS="0"
   TELTYPECUTS="1"
   DIRECTIONCUT="0"
fi
if [ $PART = "helium" ]
then
   MSCFILE=$DDIR/helium*."$ARRAY"_ID"$RECID".mscw.root
   EFFFILE=$DDIR/EffectiveAreas/
   OFIL=helium."$ARRAY"_ID"$RECID".eff
   OFFMIN=( 0. )
   OFFMAX=( 100000. )
# NOTE: this is theta and not theta2
   THETA2MIN=( 0. 1. 2. 3.0 3.5 4.0 4.5 5.0 5.5 )
   THETA2MAX=( 1. 2. 3. 3.5 4.0 4.5 5.0 5.5 6.0 )
   ISOTROPY="1"
   AZBINS="0"
   TELTYPECUTS="1"
   DIRECTIONCUT="0"
fi
NOFF=${#OFFMIN[@]}
NTH2=${#THETA2MIN[@]}
######################################################################


###############################################################################
# loop over all MC cuts
for ((i=0; i < $NOFF; i++))
do
   iMIN=${OFFMIN[$i]}
   iMAX=${OFFMAX[$i]}
# loop over all theta2 cuts
   for ((j=0; j < $NTH2; j++))
   do
     jMIN=${THETA2MIN[$j]}
     jMAX=${THETA2MAX[$j]}

###############################################################################
# theta2 cut of protons and electron should match the rings from the isotropic gammas
      if [ $PART = "proton" ] || [ $PART = "proton_onSource" ] || [ $PART = "electron" ] || [ $PART = "helium" ]
      then
         jMIN=$(echo "$jMIN*$jMIN" | bc -l )
         jMAX=$(echo "$jMAX*$jMAX" | bc -l )
      fi

###############################################################################
# create cut file
      iCBFILE=`basename $CFIL`      
      iCFIL=$FDIR/effectiveArea-CTA-$PART-$i-$j.$iCBFILE
      cp -f $CFIL $iCFIL

      sed -e "s|OFFMIN|$iMIN|" $iCFIL > $iCFIL-a
      rm -f $iCFIL
      sed -e "s|OFFMAX|$iMAX|" $iCFIL-a > $iCFIL-b
      rm -f $iCFIL-a
      sed -e "s|THETA2MIN|$jMIN|" $iCFIL-b > $iCFIL-c
      rm -f $iCFIL-b
      sed -e "s|THETA2MAX|$jMAX|" $iCFIL-c > $iCFIL-d
      rm -f $iCFIL-c
      sed -e "s|DIRECTIONCUT|$DIRECTIONCUT|" $iCFIL-d > $iCFIL-e
      rm -f $iCFIL-d
      mv -f $iCFIL-e $iCFIL
      echo $iCFIL

###############################################################################
# create run list
      MSCF=$FDIR/effectiveArea-CTA-$PART-$INPU-$i-$j.dat
      rm -f $MSCF
      echo "effective area data file for $PART $INPU $i $j" > $MSCF
###############################################################################
# general run parameters
###############################################################################
# filling mode
###############################################################################
# fill IRFs and effective areas
      if [ $PART = "gamma_onSource" ] || [ $PART = "gamma_cone10" ]
      then
	 echo "* FILLINGMODE 0" >> $MSCF
      else
	 echo "* FILLINGMODE 2" >> $MSCF
      fi
# fill IRFs only
      echo "* ENERGYRECONSTRUCTIONMETHOD $EREC" >> $MSCF
      echo "* ENERGYAXISBINS 60" >> $MSCF
      echo "* ENERGYRECONSTRUCTIONQUALITY 0" >> $MSCF
# one azimuth bin only
      echo "* AZIMUTHBINS $AZBINS" >> $MSCF
      echo "* ISOTROPICARRIVALDIRECTIONS $ISOTROPY" >> $MSCF
      echo "* TELESCOPETYPECUTS $TELTYPECUTS" >> $MSCF
# do fill analysis (a 1 would mean that MC histograms would be filled only)
      echo "* FILLMONTECARLOHISTOS 0" >> $MSCF
# spectral index
      if [ $PART = "proton" ] || [ $PART = "proton_onSource" ]
      then
         echo "* ENERGYSPECTRUMINDEX  1 2.6 0.1" >> $MSCF
      fi
      if [ $PART = "helium" ] 
      then
         echo "* ENERGYSPECTRUMINDEX  1 2.6 0.1" >> $MSCF
      fi
      if [ $PART = "electron" ] 
      then
         echo "* ENERGYSPECTRUMINDEX  1 3.0 0.1" >> $MSCF
      fi
      if [ $PART = "gamma_onSource" ] || [ $PART = "gamma_cone10" ]
      then
         echo "* ENERGYSPECTRUMINDEX  1 2.5 0.1" >> $MSCF
      fi
      echo "* CUTFILE $iCFIL" >> $MSCF
      echo "* SIMULATIONFILE_DATA $MSCFILE" >> $MSCF
      if [ $INPU = "msc" ]
      then
         echo "* SIMULATIONFILE_MC $MSCFILE" >> $MSCF
      fi
      if [ $INPU = "eff" ]
      then
#         echo "* SIMULATIONFILE_MCHISTO $EFFFILE/$OFIL-$i-$j.root" >> $MSCF
         echo "* SIMULATIONFILE_MCHISTO $CTA_USER_DATA_DIR/analysis/EffectiveArea/E/stdCuts/$OFIL-$i-$j.root" >> $MSCF
      fi

# output file
      OFIX=$ODIR/$OFIL-$i-$j

# create run script
      FNAM="CTAeffArea-$PART-$INPU-$ARRAY-$i-$j"
# run parameter file
      sed -e "s|IIIIFIL|$MSCF|" $FSCRIPT.sh > $QSHELLDIR/$FSCRIPT-1.sh
# output file
      sed -e "s|TTTTFIL|$OFIX|" $QSHELLDIR/$FSCRIPT-1.sh > $QSHELLDIR/$FSCRIPT-2.sh
      rm -f $QSHELLDIR/$FSCRIPT-1.sh
      mv -f $QSHELLDIR/$FSCRIPT-2.sh $QSHELLDIR/$FNAM.sh

      chmod u+x $QSHELLDIR/$FNAM.sh

      echo
      echo "preparing new analysis run"
      echo "--------------------------"
      echo
      echo "gamma/hadron separation file"
      echo $iCFIL
      echo "run script is $QSHELLDIR/$FNAM.sh"
      echo "batch log and error files are written to $FDIR"
      echo "parameter files write written to $FDIR"

# submit the job
      if [ $INPU = "msc" ]
      then
         echo "submitting to long queue"
         qsub -l h_cpu=40:29:00 -l h_vmem=6000M -l tmpdir_size=35G  -V -o $FDIR -e $FDIR "$QSHELLDIR/$FNAM.sh"
      fi
      if [ $INPU = "eff" ]
      then
         echo "submitting to short queue"
         qsub -l h_cpu=00:29:00 -l h_vmem=6000M -l tmpdir_size=10G  -V -o $FDIR -e $FDIR "$QSHELLDIR/$FNAM.sh"
      fi
   done
done

exit

