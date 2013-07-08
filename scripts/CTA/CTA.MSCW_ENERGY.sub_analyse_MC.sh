#!/bin/sh
#
# script to analyse CTA MC files with lookup tables
#
# Author: Gernot Maier
#


if [ $# -ne 5 ]
then
   echo
   echo "CTA.MSCW_ENERGY.sub_analyse_MC.sh <tablefile> <recid> <subarray list> <data set> <script input parameter file>"
   echo
   echo "  <tablefile>     table file name (without .root)"
   echo "                  expected file name: xxxxxx-SUBARRAY.root; SUBARRAY is added by this script"
   echo "  <recid>         reconstruction ID"
   echo "  <subarraylist > text file with list of subarray IDs"
   echo "  <particle>      gamma_onSource / gamma_cone10 / electron / proton / helium"
   echo "  <data set>      e.g. ultra, ISDC3700m, ..."
   echo "  <script input parameter file>  file with directories, etc.; see example in"
   echo "                             $CTA_EVNDISP_AUX_DIR/ParameterFiles/scriptsInput.runparameter"
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
FILEN=250

#######################################
# read values from parameter file
ANAPAR=$5
if [ ! -e $ANAPAR ]
then
  echo "error: analysis parameter file not found: $ANAPAR" 
  exit
fi
echo "reading analysis parameter from $ANAPAR"
ANADIR=`grep MSCWSUBDIRECTORY  $ANAPAR | awk {'print $2'}`
if [ -z "$ANADIR" ]
then
  echo "error: analysis parameter file not found: $ANAPAR" 
  echo "(no ANADIR given)"
  exit
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
SHELLDIR=$CTA_USER_LOG_DIR"/queueShellDir/MSCW"
mkdir -p $SHELLDIR

###########################
# particle
if [ $DSET = "prod2-G-Leoncito-North" ] || [ $DSET = "prod2-G-Leoncito-South" ]
then
   VPART=( "proton" )
else
   VPART=( "gamma_onSource" "gamma_cone10" "electron" "proton" )
fi
NPART=${#VPART[@]}

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
# input files lists

      TMPLIST=$SHELLDIR/MSCW.tmplist.list
      rm -f $TMPLIST
      echo $TMPLIST
      ls -1 $CTA_USER_DATA_DIR/analysis/AnalysisData/$DSET/$SUBAR/$PART/*.root > $TMPLIST
      echo "total number of files for particle type $PART : "
      NTMPLIST=`wc -l $TMPLIST | awk '{print $1}'`
      echo $NTMPLIST
# loop over all input files, start a job when $FILEN files are found
      for ((l = $FILEN; l < $NTMPLIST; l+=$FILEN ))
      do
# output file name for mscw_energy
	TFIL=$PART$NC"."$SUBAR"_ID"$RECID"-"$DSET"-$l.mscw"
# input file list
	IFIL=$ODIR/$TFIL.list
	rm -f $IFIL
	let "TMPL = $NTMPLIST - $l"
	if [[ "$TMPL" -lt "$FILEN" ]]
	then
	   FILEN=$TMPL
        fi
	echo $l $TMPL $FILEN
	head -n $l $TMPLIST | tail -n $FILEN > $IFIL

# skeleton script
	 FSCRIPT="CTA.MSCW_ENERGY.qsub_analyse_MC"

	 FNAM="$SHELLDIR/MSCW.ana-$DSET-ID$RECID-$PART-array$SUBAR"

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
	 qsub -l h_cpu=41:29:00 -l os="sl*" -l h_vmem=9000M -l tmpdir_size=5G  -V -j y -o $QLOG -e $QLOG "$FNAM.sh" 
	 echo "run script written to $FNAM.sh"
	 echo "queue log and error files written to $QLOG"
     done
   done
done

exit

