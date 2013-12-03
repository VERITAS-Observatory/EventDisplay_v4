#!/bin/sh
#
# script to analyse CTA MC files with lookup tables
#
# Author: Gernot Maier
#


if [ $# -lt 6 ]
then
   echo
   echo "CTA.MSCW_ENERGY.sub_analyse_MC.sh <tablefile> <recid> <subarray list> <data set> <output directory> <onSource/cone> [qsub options]"
   echo
   echo "  <tablefile>     table file name (without .root)"
   echo "                  expected file name: xxxxxx-SUBARRAY.root; SUBARRAY is added by this script"
   echo "  <recid>         reconstruction ID"
   echo "  <subarraylist > text file with list of subarray IDs"
   echo "  <particle>      gamma_onSource / gamma_cone / electron / proton / helium"
   echo "  <data set>      e.g. ultra, ISDC3700m, ..."
   echo "  <output directory> mscw files are written into this directory"
   echo
   echo "optional (for a huge amount of MC files):"
   echo "  [wildcard]     used in the < CTA.MSCW_ENERGY.subParallel_analyse_MC.sh > script"
   echo
   exit
fi

#########################################
# input parameters
TABLE=$1
RECID=$2
VARRAY=`awk '{printf "%s ",$0} END {print ""}' $3`
DSET="$4"
CONE="FALSE"
if [[ $6 == "cone" ]]
then
  CONE="TRUE"
fi
ANADIR=$5
if [ $CONE = "FALSE" ]
then
   ANADIR=$ANADIR-onAxis
fi
QSUBOPT=""
if [ -n $7 ]
then
   QSUBOPT="$7"
fi

#########################################
# checking the path for binary
if [ -z $EVNDISPSYS ]
then
    echo "no EVNDISPSYS env variable defined"
    exit
fi

#########################################
# output directory for error/output from batch system
# in case you submit a lot of scripts: QLOG=/dev/null
DATE=`date +"%y%m%d"`
# QLOG=$CTA_USER_LOG_DIR/$DATE/ANALYSETABLES/
# mkdir -p $QLOG
QLOG=/dev/null

# output directory for shell scripts
SHELLDIR=$CTA_USER_LOG_DIR/$DATE"/MSCWANA/"
mkdir -p $SHELLDIR

###########################
# particle types
VPART=( "gamma_onSource" "gamma_cone" "electron" "proton" )
NPART=${#VPART[@]}

###########################
# MC azimuth angles
VAZ=( "_0deg" "_180deg" )
NAZ=${#VAZ[@]}

#########################################
#loop over all arrays
#########################################
for SUBAR in $VARRAY
do
   echo "STARTING ARRAY $SUBAR"

# output directory
   ODIR=$CTA_USER_DATA_DIR"/analysis/AnalysisData/"$DSET"/"$SUBAR"/$ANADIR/"
   mkdir -p $ODIR

#########################################
# loop over all particle types
   for ((m = 0; m < $NPART; m++ ))
   do
      PART=${VPART[$m]}

#########################################
# loop over all MC az angles
      for ((n = 0; n < $NAZ; n++ ))
      do
         MCAZ=${VAZ[$n]}

# take $FILEN files and combine them into one mscw file
	 FILEN=500
	 if [ $PART = "proton" ]
	 then
	    FILEN=1500
	 fi

#########################################
# input files lists

	 TMPLIST=$SHELLDIR/MSCW.tmplist.list
	 rm -f $TMPLIST
	 echo $TMPLIST
	 find $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$SUBAR/$PART/ -name "*[0-9]*$MCAZ.root" > $TMPLIST
	 echo "total number of files for particle type $PART ($MCAZ) : "
	 NTMPLIST=`wc -l $TMPLIST | awk '{print $1}'`
########################################################################
# loop over all input files, start a job when $FILEN files are found
	 for ((l = 1; l < $NTMPLIST; l+=$FILEN ))
	 do
# output file name for mscw_energy
	    TFIL=$PART$NC"."$SUBAR"_ID$RECID$MCAZ-"$DSET"-$l.mscw"
# input file list
	    IFIL=$ODIR/$TFIL.list
	    rm -f $IFIL
	    let "k = $l + $FILEN"
	    sed -n "$l,$k p" $TMPLIST > $IFIL
# skeleton script
	    FSCRIPT="CTA.MSCW_ENERGY.qsub_analyse_MC"

	    FNAM="$SHELLDIR/MSCW.ana-$DSET-ID$RECID-$PART-array$SUBAR-$6"

	    sed -e "s|TABLEFILE|$TABLE|" \
		-e "s|IIIIFIL|$IFIL|" \
		-e "s|TTTTFIL|$TFIL|" \
		-e "s|RECONSTRUCTIONID|$RECID|" \
		-e "s|ARRAYYY|$SUBAR|" \
		-e "s|DATASET|$DSET|" \
		-e "s|AAAAADIR|$ANADIR|" $FSCRIPT.sh > $FNAM.sh 

	    chmod u+x $FNAM.sh
	    echo $FNAM.sh

# submit the job
	    qsub $QSUBOPT -l h_cpu=41:29:00 -l os=sl6 -l h_vmem=9000M -l tmpdir_size=5G  -V -j y -o $QLOG -e $QLOG "$FNAM.sh" 
	    echo "run script written to $FNAM.sh"
	    echo "queue log and error files written to $QLOG"
       done
     done
   done
done

exit

