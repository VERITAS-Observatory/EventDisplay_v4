#!/bin/bash
#
# script to convert sim_tel output files to EVNDISP DST file and then run eventdisplay
#
# PROD2 analysis
#
# Author: Gernot Maier
#

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.sh CTA

ILIST=SIMTELLIST
ILINE=$SGE_TASK_ID
PART=PAAART
SUBA="ARRAY"
KEEP=KEEEEEEP
ACUT=ARC
DSET=DATASET
LOGF=FLL
PEDFILE="$CTA_USER_DATA_DIR/analysis/AnalysisData/prod2-Aar/Calibration/Aar.peds.root"

# set array
FIELD=$SUBA

# converter command line parameter
COPT="-f 1 -c $PEDFILE"

# eventdisplay command line parameter
OPT="-shorttree -l2setspecialchannels nofile -writenoMCTree -reconstructionparameter $ACUT"

# set timtelarray file and cp simtelarray.gz file to TMPDIR
if [ !-e $ILIST ]
then
   echo "ERROR: list of simulation files does not exist: $ILIST"
   exit
fi

IFIL=`head -n $ILINE $ILIST | tail -n 1`
echo "$IFIL"
cp -v -f $IFIL $TMPDIR"/"

if [ ! -e $IFIL ]
then
   echo "ERROR: SIMTELFILE does not exist"
   exit
fi

# output file
OFIL=`basename $IFIL .gz`
echo "OUTPUT FILE $OFIL"

#loop over all arrays
for N in $FIELD
do
   echo "RUNNING  $N"
# output data files are written to this directory
   ODIR=$CTA_USER_DATA_DIR"/analysis/AnalysisData/"$DSET"/"$N"/"$PART"/"
   mkdir -p $ODIR

####################################################################
# execute converter
   $EVNDISPSYS/bin/CTA.convert_hessio_to_VDST $COPT -a $CTA_EVNDISP_AUX_DIR/DetectorGeometry/CTA.prod2.$N.lis -o $TMPDIR/$OFIL.root $TMPDIR/$OFIL.gz >& $TMPDIR/$OFIL.$N.convert.log

####################################################################
# execute eventdisplay
  $EVNDISPSYS/bin/evndisp -sourcefile $TMPDIR/$OFIL.root $OPT -outputdirectory $TMPDIR >& $TMPDIR/$OFIL.$N.evndisp.log

####################################################################
# move evndisp files to data directory
   if [ $KEEP == "1" ]
   then
      mkdir -p $ODIR/VDST
      cp -v -f $TMPDIR/$OFIL.root $ODIR/VDST/
   fi
   rm -f -v $TMPDIR/$OFIL.root
   cp -v -f $TMPDIR/*.root $ODIR
done

# tar the log files
cd $TMPDIR
tar -czvf $OFIL.tar.gz *.log
mkdir -p $CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET/LOGFILES-$LOGF
mv -v -f $OFIL.tar.gz $CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET/LOGFILES-$LOGF/

exit

