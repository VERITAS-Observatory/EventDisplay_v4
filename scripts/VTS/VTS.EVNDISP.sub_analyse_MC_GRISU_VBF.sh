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
WOFF=( 0.5 )
WWOF=( 050 )
WOFF=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
WWOF=( 050 000 025 075 100 125 150 175 200 )
WOFF=( 0.00 )
WWOF=( 000 )
NWOFF=${#WOFF[@]}
# NOISE levels
NOISE=( 200 250 075 100 150 325 425 550 750 1000 )
NOISE=( 250 1000 )
NNOIS=${#NOISE[@]}

################################################
# loop over all wobble offsets and noise level
################################################
for (( k = 0; k < $NWOFF; k++ ))
do
   WOB=${WOFF[$k]}
   WOG=${WWOF[$k]}

   for (( n = 0; n < $NNOIS; n++ ))
   do
     NOIS=${NOISE[$n]}

###############################################################################################################
# target directory for simulation output files
# run scripts are written to this directory
###############################################################################################################
     DSET="grisu"
     if [ $PART = "1" ]
     then
	 DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/$DSET/ATM$ATMO/"
	 ODIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/gamma_"$ZEW"deg_750m/wobble_$WOB/"
     fi 
     if [ $PART = "14" ]
     then
	 DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/vbf/"
	 ODIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/proton_"$ZEW"deg_750m/wobble_$WOB/"
     fi
     if [ $PART = "402" ]
     then
	 DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/vbf/"
	 ODIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/helium_"$ZEW"deg_750m/wobble_$WOB/"
     fi

##################################################################
# making run scripts
##################################################################
   FDIR=$VERITAS_USER_LOG_DIR"/queueShellDir/"
   if [ ! -d $FDIR ]
   then
     mkdir -p $FDIR
   fi
   QLOGDIR=$FDIR
   echo "DATA DIR: $ODIR"
   echo "SHELL AND LOG DIR $QLOGDIR"

#   if [   ]
#   then
#     QLOGDIR="/dev/null"
#   fi

   CSCRIPT="VTS.EVNDISP.qsub_analyse_MC_GRISU_VBF"
   OSCRIPT="qsub_evndisp_MC_VBF-$ZEW-$WOB-$NOIS-$ATMO"

# set zenith angle
    sed -e "s/123456789/$ZEW/" $CSCRIPT.sh  > $FDIR/$OSCRIPT-b.sh

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
    sed -e "s/IDIDID/$PART/"  $FDIR/$OSCRIPT-t.sh > $FDIR/$OSCRIPT.sh
    rm -f $FDIR/$OSCRIPT-t.sh

    chmod u+x $FDIR/$OSCRIPT.sh

# submit the job
   qsub -l os="sl*" -V -l h_cpu=11:29:00 -l h_vmem=6000M -l tmpdir_size=100G  -o $QLOGDIR/ -e $QLOGDIR/ "$FDIR/$OSCRIPT.sh"

  done

done

exit
