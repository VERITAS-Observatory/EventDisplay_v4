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
PEDFILE=PPPP
TRGMASKDIR=TRIGGGG

# set array
FIELD=$SUBA

# converter command line parameter
COPT="-f 1 -c $PEDFILE"

# eventdisplay command line parameter
OPT="-averagetzerofiducialradius=0.5 -shorttree -l2setspecialchannels nofile -writenoMCTree -reconstructionparameter $ACUT"

# set timtelarray file and cp simtelarray.gz file to TMPDIR
if [ ! -e $ILIST ]
then
   echo "ERROR: list of simulation files does not exist: $ILIST"
   exit
fi

echo "getting line $ILINE from list"
echo "list: $ILIST"
IFIL=`head -n $ILINE $ILIST | tail -n 1`
echo "DATA FILE $IFIL"
cp -v -f $IFIL $TMPDIR"/"
# log file directory
DATE=`date +"%y%m%d"`
mkdir -p $CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET/LOGFILES-$DATE-$LOGF

if [ ! -e $IFIL ]
then
   echo "ERROR: SIMTELFILE does not exist"
   exit
fi

#####################################
# output file
OFIL=`basename $IFIL .gz`
echo "OUTPUT FILE $OFIL"

#####################################
# trigmask file (optional)
if [ $TRGMASKDIR != "FALSE" ]
then
   if [[ $DSET == *Leoncito* ]] || [[ $DSET == *Aar* ]]
   then
      FFIL=`basename $TMPDIR/$OFIL.gz .simtel.gz`
      echo "FIL $FFIL"
   elif [[ $DSET == *SAC* ]]
   then
      if [[ $PART == "gamma_cone" ]] && [[  $DSET == *SAC084* ]]
      then
	FFIL=`basename $TMPDIR/$OFIL.gz ___cta-prod2_desert-SACx0.84_cone10.simtel.gz`
      else
	FFIL=`basename $TMPDIR/$OFIL.gz .simtel.gz`
      fi
   fi
   TRIGF=`find $TRGMASKDIR -name $FFIL*trgmask*`
   if [ -n "$TRIGF" ] && [ -e "$TRIGF" ]
   then
      COPT="$COPT -t $TRIGF"
      echo "CONVERTER OPTIONS: $COPT"
   else
      ELOG="$CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET/LOGFILES-$DATE-$LOGF/$OFIL.error.trg.log"
      rm -f $ELOG
      touch $ELOG
      echo "COULD NOT FIND TRGMASK FILE for SIMTEL FILE " $IFIL >> $ELOG
      echo "search directory: $TRGMASKDIR" >> $ELOG
      echo "search string: $FFIL" >> $ELOG
      echo "found: $TRIGF" >> $ELOG
      echo "error, cannot analyse this file..." >> $ELOG
      exit
   fi
fi

####################################################################
# loop over all arrays
for N in $FIELD
do
# remove spaces
   N=`echo $N | tr -d ' '`
   echo "RUNNING _"$N"_"
# output data files are written to this directory
   ODIR=$CTA_USER_DATA_DIR"/analysis/AnalysisData/"$DSET"/"$N"/"$PART"/"
   mkdir -p $ODIR

####################################################################
# execute converter
   $EVNDISPSYS/bin/CTA.convert_hessio_to_VDST $COPT -a $CTA_EVNDISP_AUX_DIR/DetectorGeometry/CTA.prod2$N.lis -o $TMPDIR/$OFIL.root $TMPDIR/$OFIL.gz >& $TMPDIR/$OFIL.$N.convert.log

####################################################################
# execute eventdisplay
  $EVNDISPSYS/bin/evndisp -sourcefile $TMPDIR/$OFIL.root $OPT -outputdirectory $TMPDIR >& $TMPDIR/$OFIL.$N.evndisp.log

####################################################################
# get runnumber and azimuth
  MCAZ=`$EVNDISPSYS/bin/printRunParameter $TMPDIR/$OFIL.root -mcaz`
  RUNN=`$EVNDISPSYS/bin/printRunParameter $TMPDIR/$OFIL.root -runnumber`

####################################################################
# move dst (if required ) and evndisp files to data directory
   if [ $KEEP == "1" ]
   then
      mkdir -p $ODIR/VDST
      cp -v -f $TMPDIR/$OFIL.root $ODIR/VDST/
   fi
   rm -f -v $TMPDIR/$OFIL.root
   cp -v -f $TMPDIR/$RUNN.root $ODIR/$RUNN"_"$MCAZ"deg.root"
done

####################################################################
# tar the log files
cd $TMPDIR
tar -czvf $OFIL.tar.gz *.log
mv -v -f $OFIL.tar.gz $CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET/LOGFILES-$DATE-$LOGF/

exit
