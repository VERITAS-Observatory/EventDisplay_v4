#! /bin/bash
#
# submit evndisp for simulations (CARE)
#
# Gernot Maier 
#
#
MACHINE=`hostname`
MCD=`date`

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ] || [ ! -n "$4" ] || [ ! -n "$5" ] || [ ! -n "$6" ]
then
  echo "VTS.EVNDISP.sub_analyse_MC_CARE_VBF.sh <ze> <array=V4/V5/V6> <summation window> <particle=1/2/14/402> <run nunmber> <noise>"
  echo "V4: array before T1 move (before Autumn 2009)"
  echo "V5: array after T1 move (from Autumn 2009)"
  echo "V6: array after camera upgrade (from Autumn 2012)"
  exit
fi

###############################################
# read input parameters
################################################
ZEW=$1
ARRAY=$2
SW=$3
PART=$4
METH="LL"
RUN=$5
ATMO="06"
WOB="0.5"
WOG="050"
NOISE=$6

###############################################################################################################
# in- and output directories for simulation 
# run scripts are written to this directory
###############################################################################################################
if [ $PART = "1" ]
then
   DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/care_optics_Nov10/ATM$ATMO/"
   ODIR=$VERITAS_DATA_DIR"/analysis/"$ARRAY"_FLWO/care_optics_Nov10/gamma_"$ZEW"deg_750m/wobble_$WOB/"
fi 
if [ $PART = "2" ]
then
   DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/care_optics_Nov10/ATM$ATMO/"
   ODIR=$VERITAS_DATA_DIR"/analysis/"$ARRAY"_FLWO/care_optics_Nov10/electron_"$ZEW"deg_750m/wobble_$WOB/"
fi
if [ $PART = "14" ]
then
   DDIR=$VERITAS_DATA_DIR"/simulations/"$ARRAY"_FLWO/care_optics_Nov10/ATM$ATMO/"
   ODIR=$VERITAS_DATA_DIR"/analysis/"$ARRAY"_FLWO/care_optics_Nov10/proton_"$ZEW"deg_750m/wobble_$WOB/"
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
OSCRIPT="qsub_evndisp_MC_CARE_VBF-$ZEW-$WOB-$SW-$NOIS-$METH-$ATMO"

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
sed -e "s/NOISENOISE/$NOISE/" $FDIR/$OSCRIPT-e.sh > $FDIR/$OSCRIPT-r.sh
rm -f $FDIR/$OSCRIPT-e.sh

# summation window
sed -e "s/SUMWINDOW/$SW/" $FDIR/$OSCRIPT-r.sh > $FDIR/$OSCRIPT-s.sh
rm -f $FDIR/$OSCRIPT-r.sh

# V4 or V5?
sed -e "s/XXXXXX/$ARRAY/" $FDIR/$OSCRIPT-s.sh > $FDIR/$OSCRIPT-t.sh
rm -f $FDIR/$OSCRIPT-s.sh

# run number
sed -e "s/UUUAAA/$RUN/" $FDIR/$OSCRIPT-t.sh > $FDIR/$OSCRIPT-u.sh
rm -f $FDIR/$OSCRIPT-t.sh

# gamma/proton/etc
sed -e "s/IDIDID/$PART/"  $FDIR/$OSCRIPT-u.sh > $FDIR/$OSCRIPT.sh
rm -f $FDIR/$OSCRIPT-u.sh

chmod u+x $FDIR/$OSCRIPT.sh
echo "RUNSCRIPT $FDIR/$OSCRIPT.sh"
echo "QFILES $QLOGDIR/"
echo "LOG AND DATA FILES: $ODIR"

# submit the job
if [ $METH = "GEO" ]
then
  qsub -V -l h_cpu=10:00:00 -l tmpdir_size=100G  -o $QLOGDIR/ -e $QLOGDIR/ "$FDIR/$OSCRIPT.sh"
fi
if [ $METH = "LL" ]
then
  qsub -V -l h_cpu=11:49:00 -l tmpdir_size=100G  -o $QLOGDIR/ -e $QLOGDIR/ "$FDIR/$OSCRIPT.sh"
fi

exit
