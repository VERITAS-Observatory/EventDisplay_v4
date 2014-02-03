#! /bin/bash
#
# submit evndisp for simulations (CARE)
#
# Gernot Maier 
#
#
MACHINE=`hostname`
MCD=`date`

if [ $# -lt 2 ]
then
  echo
  echo "VTS.EVNDISP.sub_analyse_MC_CARE_VBF.sh <ze> <array=V4/V5/V6>"
  echo
  echo "  analyse VTS simulations created with CARE (VBF format)"
  echo
  echo "    V4: array before T1 move (before Autumn 2009)"
  echo "    V5: array after T1 move (from Autumn 2009)"
  echo "    V6: array after camera upgrade (from Autumn 2012)"
  echo
  exit
fi

###############################################
# read input parameters
################################################
ZEW=$1
ARRAY=$2
RUN="1200"
ATMO="21"
WOB="0.5"

###############################################
# input parameters
################################################
ACUT="EVNDISP.reconstruction.runparameter"
# particle (1,2,14,402)
PART="1"
# NOISE levels
NOISE=( 50 80 120 170 230 290 370 450 )
NNOIS=${#NOISE[@]}

###############################################################################################################
# in- and output directories for simulation 
# run scripts are written to this directory
###############################################################################################################
DSET="care_Jan1427"
if [ $PART = "1" ]
then
   DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/$DSET/"
   ODIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/$DSET/gamma_"$ZEW"deg_750m/wobble_$WOB/"
fi 
if [ $PART = "2" ]
then
   DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/$DSET/ATM$ATMO/"
   ODIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/$DSET/electron_"$ZEW"deg_750m/wobble_$WOB/"
fi
if [ $PART = "14" ]
then
   DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/$DSET/ATM$ATMO/"
   ODIR=$VERITAS_DATA_DIR"/analysis/EVDv400/"$ARRAY"_FLWO/$DSET/proton_"$ZEW"deg_750m/wobble_$WOB/"
fi
echo $DDIR

##################################################################
# making run scripts
##################################################################
FDIR=$VERITAS_USER_LOG_DIR"/queueShellDir/"
if [ ! -d $FDIR ]
then
  mkdir -p $FDIR
fi
QLOGDIR=$FDIR

CSCRIPT="VTS.EVNDISP.qsub_analyse_MC_CARE_VBF"

# loop over all noise level
for (( n = 0; n < $NNOIS; n++ ))
do
  NOIS=${NOISE[$n]}
  echo "NOISE $n $NOIS"
  OSCRIPT="$ARRAY-EV_CARE_VBF-$ZEW-$WOB-$NOIS-$ATMO"

# set zenith angle
# add data directory
# atmosphere
# add output directory
# set wobble offset
# set noise level
# array analysis acut (run parameters)
# epoch
# run number
# gamma/proton/etc
    sed -e "s/123456789/$ZEW/" \
        -e "s|DATADIR|$DDIR|" \
        -e "s|AAAAAAAA|$ATMO|" \
        -e "s|OUTDIR|$ODIR|" \
        -e "s/987654321/$WOB/" \
        -e "s/NOISENOISE/$NOIS/" \
        -e "s|ACUT|$ACUT|" \
        -e "s/XXXXXX/$ARRAY/" \
        -e "s/UUUAAA/$RUN/" \
        -e "s/IDIDID/$PART/"  $CSCRIPT.sh > $FDIR/$OSCRIPT.sh

# prepare and submit the job
    chmod u+x $FDIR/$OSCRIPT.sh
    echo "RUNSCRIPT $FDIR/$OSCRIPT.sh"
    echo "QFILES $QLOGDIR/"
    echo "LOG AND DATA FILES: $ODIR"

# submit the job
    echo $FDIR/$OSCRIPT.sh
    qsub -V -l os=sl6 -l h_cpu=48:49:00 -l tmpdir_size=250G -l h_vmem=4G -o $QLOGDIR/ -e $QLOGDIR/ "$FDIR/$OSCRIPT.sh"
done

exit
