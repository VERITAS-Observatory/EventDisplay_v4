#! /bin/bash
#
# submit evndisp for grisu simulations (analyse all noise levels at the same time)
#
#
MACHINE=`hostname`
MCD=`date`

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ] || [ ! -n "$5" ] 
then
  echo
  echo "./VTS.EVNDISP.sub_evndisp_MC_GRISU_VBF.sh <ze> <array=V4/V5/V6> <particle=1/14/402> <ATMOSPHERE=21/22> <Number of events per division>" 
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

NEVENTS=5000000
if [ -n "$5" ]
then
  NEVENTS=$5
fi

################################################
# define arrays in wobble offset and noise level
################################################
# wobble offsets
WOFF=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
WWOF=( 050 000 025 075 100 125 150 175 200 )
##WOFF=( 0.5 )
##WWOF=( 050 )
NWOFF=${#WOFF[@]}
# NOISE levels
NOISE=( 200 075 100 150 250 325 425 550 750 1000 )
##NOISE=( 200 )
NNOIS=${#NOISE[@]}

NFIL="0"
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
	 ODIR=$VERITAS_USER_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/newtemplate/gamma_"$ZEW"deg_750m/wobble_$WOB/"
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
   ##QLOGDIR=$FDIR/
   QLOGDIR=/dev/null

   echo "DATA DIR: $ODIR"
   echo "SHELL AND LOG DIR $FDIR $QLOGDIR"

   CSCRIPT="VTS.EVNDISP.qsub_analyse_MC_Frogs"
   OSCRIPT="qsub_evndisp_MC_VBF_Frogs-$ZEW-$WOB-$NOIS-$ATMO-$f"

# file number (useful for protons only)
    sed -e "s/NFINFIL/$f/" $CSCRIPT.sh  > $FDIR/$OSCRIPT-a.sh

# run number
    sed -e "s/RUNRUNRUN/$NRUN/" $FDIR/$OSCRIPT-a.sh > $FDIR/$OSCRIPT-aa.sh
    rm -f $FDIR/$OSCRIPT-a.sh

# set zenith angle
    sed -e "s/123456789/$ZEW/"  $FDIR/$OSCRIPT-aa.sh > $FDIR/$OSCRIPT-b.sh
    rm -f $FDIR/$OSCRIPT-aa.sh

# add data directory
    sed -e "s|DATADIR|$DDIR|" $FDIR/$OSCRIPT-b.sh > $FDIR/$OSCRIPT-c.sh
    rm -f $FDIR/$OSCRIPT-b.sh

# atmosphere
    sed -e "s|AAAAAAAA|$ATMO|" $FDIR/$OSCRIPT-c.sh > $FDIR/$OSCRIPT-d.sh
    rm -f $FDIR/$OSCRIPT-c.sh

# add output directory
    sed -e "s|OUTDIR|$ODIR|" $FDIR/$OSCRIPT-d.sh > $FDIR/$OSCRIPT-c.sh
    rm -f $FDIR/$OSCRIPT-d.sh

# set wobble offset
    sed -e "s/987654321/$WOB/" $FDIR/$OSCRIPT-c.sh > $FDIR/$OSCRIPT-d.sh
    rm -f $FDIR/$OSCRIPT-c.sh

# set wobble offset for vbf file
    sed -e "s/WOGWOG/$WOG/" $FDIR/$OSCRIPT-d.sh > $FDIR/$OSCRIPT-e.sh
    rm -f $FDIR/$OSCRIPT-d.sh

# set noise level
    sed -e "s/NOISENOISE/$NOIS/" $FDIR/$OSCRIPT-e.sh > $FDIR/$OSCRIPT-r.sh
    rm -f $FDIR/$OSCRIPT-e.sh

# V4 or V5?
    sed -e "s/XXXXXX/$ARRAY/" $FDIR/$OSCRIPT-r.sh > $FDIR/$OSCRIPT-t.sh
    rm -f $FDIR/$OSCRIPT-r.sh
# gamma/proton
    sed -e "s/IDIDID/$PART/"  $FDIR/$OSCRIPT-t.sh > $FDIR/$OSCRIPT-u.sh
    rm -f $FDIR/$OSCRIPT-t.sh
# number of events to do
    sed -e "s/EVENTSEVENTS/$NEVENTS/"  $FDIR/$OSCRIPT-u.sh > $FDIR/$OSCRIPT.sh
    rm -f $FDIR/$OSCRIPT-u.sh
# Iteration
    #sed -e "s/ITERITER/$ITER/"  $FDIR/$OSCRIPT-u.sh > $FDIR/$OSCRIPT-v.sh
    #rm -f $FDIR/$OSCRIPT-u.sh

    chmod u+x $FDIR/$OSCRIPT.sh

# submit the job
   qsub -l os=sl6 -V -l h_cpu=47:59:00 -l h_vmem=6000M -l tmpdir_size=100G  -t 1-10 -o $QLOGDIR -e $QLOGDIR "$FDIR/$OSCRIPT.sh"

  done

done

    let "NRUN = $NRUN + 1"
done

exit
