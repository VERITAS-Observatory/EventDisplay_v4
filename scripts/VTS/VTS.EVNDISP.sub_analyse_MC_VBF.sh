#! /bin/bash
#
# submit evndisp for simulations (analyse all noise levels at the same time)
#
# Gernot Maier 
#
# Revision $Id: mm_sub_evndisp_MC_VBF.sh,v 1.1.2.3.2.4.6.3 2011/04/06 12:22:24 gmaier Exp $
#
MACHINE=`hostname`
MCD=`date`

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ] || [ ! -n "$5" ] || [ ! -n "$6" ]
then
  echo "VTS.EVNDISP.sub_evndisp_MC_VBF.sh <ze> <array=V4/V5> <summation window> <particle=1/14/402> <method=GEO/LL> <ATMOSPHERE=06/20/21/22>" 
  echo "V4: array before T1 move (before Autumn 2009)"
  echo "V5: array after T1 move (from Autumn 2009)"
  exit
fi

###############################################
# read input parameters
################################################
ZEW=$1
ARRAY=$2
SW=$3
PART=$4
METH=$5
ATMO=$6

################################################
# define arrays in wobble offset and noise level
################################################
# wobble offsets
WOFF=( 0.5 0.00 0.25 0.75 1.00 1.25 1.50 1.75 2.00 )
WWOF=( 050 000 025 075 100 125 150 175 200 )
WOFF=( 0.5 )
WWOF=( 050 )
NWOFF=${#WOFF[@]}
# NOISE levels
NOISE=( 200 250 075 100 150 325 425 550 750 1000 )
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
     if [ $PART = "1" ]
     then
	 DDIR=$VERITAS_DATA_DIR"simulations/"$ARRAY"_FLWO/grisu/ATM$ATMO/"
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

   CSCRIPT="VTS.EVNDISP.qsub_analyse_MC_VBF"
   OSCRIPT="qsub_evndisp_MC_VBF-$ZEW-$WOB-$SW-$NOIS-$METH-$ATMO"

# set zenith angle
    sed -e "s/123456789/$ZEW/" $CSCRIPT.sh  > $FDIR/$OSCRIPT-b.sh

# add data directory
    sed -e "s|DATADIR|$DDIR|" $FDIR/$OSCRIPT-b.sh > $FDIR/$OSCRIPT-c.sh
    rm -f $FDIR/$OSCRIPT-b.sh

# atmosphere
    sed -e "s|AAAAAAAA|$ATMO|" $FDIR/$OSCRIPT-c.sh > $FDIR/$OSCRIPT-d.sh
    rm -f $FDIR/$OSCRIPT-c.sh

# reconstruction method
    sed -e "s|RECMETH|$METH|" $FDIR/$OSCRIPT-d.sh > $FDIR/$OSCRIPT-f.sh
    rm -f $FDIR/$OSCRIPT-d.sh

# add output directory
    sed -e "s|OUTDIR|$ODIR|" $FDIR/$OSCRIPT-f.sh > $FDIR/$OSCRIPT-c.sh
    rm -f $FDIR/$OSCRIPT-f.sh

# set wobble offset
    sed -e "s/987654321/$WOB/" $FDIR/$OSCRIPT-c.sh > $FDIR/$OSCRIPT-d.sh
    rm -f $FDIR/$OSCRIPT-c.sh

# set wobble offset for vbf file
    sed -e "s/WOGWOG/$WOG/" $FDIR/$OSCRIPT-d.sh > $FDIR/$OSCRIPT-e.sh
    rm -f $FDIR/$OSCRIPT-d.sh

# set noise level
    sed -e "s/NOISENOISE/$NOIS/" $FDIR/$OSCRIPT-e.sh > $FDIR/$OSCRIPT-r.sh
    rm -f $FDIR/$OSCRIPT-e.sh

# summation window
    sed -e "s/SUMWINDOW/$SW/" $FDIR/$OSCRIPT-r.sh > $FDIR/$OSCRIPT-s.sh
    rm -f $FDIR/$OSCRIPT-r.sh

# V4 or V5?
    sed -e "s/XXXXXX/$ARRAY/" $FDIR/$OSCRIPT-s.sh > $FDIR/$OSCRIPT-t.sh
    rm -f $FDIR/$OSCRIPT-s.sh
# gamma/proton
    sed -e "s/IDIDID/$PART/"  $FDIR/$OSCRIPT-t.sh > $FDIR/$OSCRIPT.sh
    rm -f $FDIR/$OSCRIPT-t.sh

    chmod u+x $FDIR/$OSCRIPT.sh

# submit the job
   if [ $METH = "GEO" ]
   then
      qsub -V -l h_cpu=10:00:00 -l tmpdir_size=100G  -o $QLOGDIR/ -e $QLOGDIR/ "$FDIR/$OSCRIPT.sh"
   fi
   if [ $METH = "LL" ]
   then
      qsub -l os="sl*" -V -l h_cpu=40:00:00 -l h_vmem=6000M -l tmpdir_size=100G  -o $QLOGDIR/ -e $QLOGDIR/ "$FDIR/$OSCRIPT.sh"
   fi

  done

done

exit
