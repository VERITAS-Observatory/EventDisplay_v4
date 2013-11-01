#! /bin/bash
#
# submit evndisp for grisu simulations (analyse all noise levels at the same time)
#
#
MACHINE=`hostname`
MCD=`date`

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ] 
then
  echo
  echo "./VTS.EVNDISP.sub_evndisp_MC_GRISU_VBF.sh <ze> <array=V4/V5/V6> <particle=1/14/402> <ATMOSPHERE=21/22>" 
  echo 
  echo " analyse VTS simulations created with grisudet (VBF format)"
  echo
  echo "   V4: array before T1 move (before Autumn 2009)"
  echo "   V5: array after T1 move (from Autumn 2009)"
  echo "   V6: array after camera update (from Autumn 2012)"
  echo 
  exit
fi

###############################################
# read input parameters
################################################
ZEW=$1
ARRAY=$2
PART=$3
ATMO=$4

################################################
# define arrays in wobble offset and noise level
################################################
# wobble offsets
WOFF=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
WWOF=( 050 000 025 075 100 125 150 175 200 )
NWOFF=${#WOFF[@]}
# NOISE levels
NOISE=( 200 250 075 100 150 325 425 550 750 1000 )
NNOIS=${#NOISE[@]}

NFIL="1"
if [ $PART = "14" ]
then
   NFIL="28"
fi
NRUN=65000
################################################
# loop over files of same type
################################################
for (( f = 0; f <= $NFIL; f++ ))
do
   echo "FILE $f"
################################################
# loop over all wobble offsets and noise level
################################################
for (( k = 0; k < $NWOFF; k++ ))
do
   WOB=${WOFF[$k]}
   WOG=${WWOF[$k]}
   echo "WOF $k $WOB $WOB"

   for (( n = 0; n < $NNOIS; n++ ))
   do
     NOIS=${NOISE[$n]}
     echo "NOISE $n $NOIS"

###############################################################################################################
# target directory for simulation output files
# run scripts are written to this directory
###############################################################################################################
     DSET="grisu"
     if [ $PART = "1" ]
     then
	 DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/$DSET/ATM$ATMO/"
	 ODIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/gamma_"$ZEW"deg_750m/wobble_$WOB/"
	 ODIR="/lustre/fs9/group/cta/users/maierg/VERITAS/analysis/EVDv400/"$ARRAY"_FLWO/gamma_"$ZEW"deg_750m/wobble_$WOB/"
	 mkdir -p $ODIR
     fi 
     if [ $PART = "14" ]
     then
	 DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/vbf/"
	 DDIR=/lustre/fs5/group/cta/VERITAS/simulations/"$ARRAY"_FLWO/grisu/ATM21/proton_1000mScat_"$ARRAY"_Oct2012_PrePostUpgradeCompare_20130423_V420_ATM21_20deg_050wob
	 ODIR="/lustre/fs9/group/cta/users/maierg/VERITAS/analysis/EVDv400/"$ARRAY"_FLWO/proton_"$ZEW"deg_750m/wobble_$WOB/"
	 NFIL=28
     fi
     if [ $PART = "402" ]
     then
	 DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/vbf/"
	 ODIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/helium_"$ZEW"deg_750m/wobble_$WOB/"
     fi

##################################################################
# making run scripts
##################################################################
   DATE=`date +"%y%m%d"`
   FDIR=$VERITAS_USER_LOG_DIR"/"$DATE/EVNDISP.ANAMCVBF
   if [ ! -d $FDIR ]
   then
     mkdir -p $FDIR
   fi
   QLOGDIR=$FDIR
   echo "DATA DIR: $ODIR"
   echo "SHELL AND LOG DIR $QLOGDIR"

   CSCRIPT="VTS.EVNDISP.qsub_analyse_MC_GRISU_VBF"
   OSCRIPT="qsub_evndisp_MC_VBF-$ZEW-$WOB-$NOIS-$ATMO-$f"

# file number (useful for protons only)
    echo $CSCRIPT.sh
    echo  $FDIR/$OSCRIPT.sh
# run number
# set zenith angle
# add data directory
# atmosphere
# add output directory
# set wobble offset
# set wobble offset for vbf file
# set noise level
# V4, V5 or V6?
# gamma/proton
    sed -e "s|NFINFIL|$f|" \
        -e "s|RUNRUNRUN|$NRUN|" \
        -e "s|123456789|$ZEW|" \
        -e "s|DATADIR|$DDIR|" \
        -e "s|AAAAAAAA|$ATMO|" \
        -e "s|OUTDIR|$ODIR|" \
        -e "s|987654321|$WOB|" \
        -e "s|WOGWOG|$WOG|" \
        -e "s|NOISENOISE|$NOIS|" \
        -e "s|XXXXXX|$ARRAY|" \
        -e "s|IDIDID|$PART|" $CSCRIPT.sh > $FDIR/$OSCRIPT.sh

    chmod u+x $FDIR/$OSCRIPT.sh

# submit the job
   qsub -l os=sl6 -V -l h_cpu=41:29:00 -l h_vmem=6000M -l tmpdir_size=100G  -o $QLOGDIR/ -e $QLOGDIR/ "$FDIR/$OSCRIPT.sh"

  done

done

    let "NRUN = $NRUN + 1"
done

exit
