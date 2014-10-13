#!/bin/bash
#
# script to merge, convert sim_telarray files to EVNDISP DST file and then run eventdisplay
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
PPOP=UUUU

# set array
FIELD=$SUBA

###################################
# converter command line parameter
# settings for increased NSB
# COPT="-NSB 1.5 -f 1 -c $PEDFILE"
# default settings
COPT="-f 1 -c $PEDFILE"

# eventdisplay command line parameter
OPT="-averagetzerofiducialradius=0.5 -shorttree -l2setspecialchannels nofile -writenoMCTree -reconstructionparameter $ACUT $PPOP"

# set simtelarray file and cp simtelarray.gz file to TMPDIR
if [ ! -e $ILIST ]
then
   echo "ERROR: list of simulation files does not exist: $ILIST"
   exit
fi

################################
# sim_telarray files 

# string in file 1
IDFIL1="prod2_desert"
# string in file 2
IDFIL2="prod2-sc-sst-x_desert"

# first file
IFIL1=`head -n $ILINE $ILIST | tail -n 1`
echo "sim_telarray DATA FILE 1: "
echo $IFIL1
if [ ! -e $IFIL1 ]
then
    echo "ERROR: sim_telarray file does not exit:"
    echo $IFIL1
    exit
fi
# second file 
IFIL2=${IFIL1/$IDFIL1/$IDFIL2}
echo "sim_telarray DATA FILE 2: "
echo $IFIL2
if [ ! -e $IFIL2 ]
then
    echo "ERROR: sim_telarray file does not exit:"
    echo $IFIL2
    exit
fi

################################
# copy files on temporary disk
echo
echo "COPYING FILES TO $TMPDIR"
# check if files are on local disc or on dCache
# (note: DESY only, this can not handle mixed lists)
for F in $IFIL1 $IFIL2 
do
    G=`basename $F`
    if [[ $F = *acs* ]]
    then
      export DCACHE_CLIENT_ACTIVE=1
      dccp $F $TMPDIR"/"$G
    else
      cp -v -f $F $TMPDIR"/"
    fi
done
G=`basename $IFIL1`
IFIL1=$TMPDIR"/"$G
G=`basename $IFIL2`
IFIL2=$TMPDIR"/"$G
echo "files in $TMPDIR"
echo $IFIL1
echo $IFIL2

###############################
# log file directory
DATE=`date +"%y%m%d"`
mkdir -p $CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET/LOGFILES-$DATE-$LOGF
echo "Logfile directory "
echo $CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET/LOGFILES-$DATE-$LOGF

#####################################
# output file
OFIL=`basename $IFIL1 .gz`
echo
echo "OUTPUT FILE $OFIL"

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
# execute merger
   SIMFIL="$TMPDIR/mergedFile.simtel.gz"
   $HESSIOSYS/bin/merge_simtel $CTA_EVNDISP_AUX_DIR/DetectorGeometry/CTA.prod2$N.merge.lis $IFIL1 $IFIL2 $SIMFIL >& $TMPDIR/$OFIL.$N.merge.log
   
####################################################################
# execute converter
   echo "TMPDIR FILES " $SIMFIL
   $EVNDISPSYS/bin/CTA.convert_hessio_to_VDST $COPT -a $CTA_EVNDISP_AUX_DIR/DetectorGeometry/CTA.prod2$N.lis -o $TMPDIR/$OFIL.root $SIMFIL >& $TMPDIR/$OFIL.$N.convert.log

####################################################################
# execute eventdisplay
  $EVNDISPSYS/bin/evndisp -sourcefile $TMPDIR/$OFIL.root $OPT -outputdirectory $TMPDIR >& $TMPDIR/$OFIL.$N.evndisp.log

####################################################################
# get runnumber and azimuth and rename output files
  MCAZ=`$EVNDISPSYS/bin/printRunParameter $TMPDIR/$OFIL.root -mcaz`
  RUNN=`$EVNDISPSYS/bin/printRunParameter $TMPDIR/$OFIL.root -runnumber`
  cp -v -f $TMPDIR/[0-9]*.root $ODIR/$RUNN"_"$ILINE"_"$MCAZ"deg.root"

####################################################################
# move dst (if required) and evndisp files to data directory
   if [ "$KEEP" == "1" ]
   then
      mkdir -p $ODIR/VDST
      cp -v -f $TMPDIR/$OFIL.root $ODIR/VDST/
   fi
   ls -lh $TMPDIR/*.root
# clean up 
   rm -f $TMPDIR/$OFIL.root
   rm -f $TMPDIR/[0-9]*.root
   rm -f $SIMFIL
   echo "==================================================================="
   echo
done

####################################################################
# tar the log files
cd $TMPDIR
tar -czvf $OFIL.tar.gz *.log
mv -v -f $OFIL.tar.gz $CTA_USER_LOG_DIR"/analysis/AnalysisData/"$DSET/LOGFILES-$DATE-$LOGF/

exit
