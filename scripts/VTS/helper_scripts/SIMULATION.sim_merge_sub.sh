#!/bin/bash
#$ -l os=sl6
#$ -l h_cpu=00:29:00
#$ -l h_vmem=4G
#$ -l tmpdir_size=30G
#$ -j y
#

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS
source $EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_functions.sh

#SIMTRACKFILE=TRACKTRACKTRACK
MERGETRACKFILE=MEERRGG
if [ ! -f $MERGETRACK ] ; then
    echo "error, MERGETRACKFILE '$MERGETRACKFILE' doesn't exist, exiting..."
    exit 1
fi

# get to the temp dir
cp $MERGETRACKFILE $TMPDIR/
cd $TMPDIR
MERGETRACKFILE="$TMPDIR/`basename $MERGETRACKFILE`"

LINENUMBER=$SGE_TASK_ID

MERGETRACKLINE=$( sed $LINENUMBER'q;d' $MERGETRACKFILE )
echo "Loading line: '$MERGETRACKLINE'"
echo "$MERGETRACKLINE" | tr '+' '\n' | awk '{ printf "  %s\n", $0 }'

MERGELIST=$(    getTagArgFromTagline "mergelist"    "$MERGETRACKLINE" )
MERGEOUTPUT=$(  getTagArgFromTagline "mergeoutput"  "$MERGETRACKLINE" )
MERGELOG=$(     getTagArgFromTagline "mergelog"     "$MERGETRACKLINE" )
FIRSTCORSIKA=$( getTagArgFromTagline "firstcorsika" "$MERGETRACKLINE" )

#echo "NKH MERGELIST '$MERGELIST'"
#echo "NKH MERGEOUTPUT '$MERGEOUTPUT'"
#echo "NKH MERGELOG '$MERGELOG'"
#echo "NKH FIRSTCORSIKA'$FIRSTCORSIKA'"

# prepare the output directory
OUTPUTDIR=$( dirname $MERGEOUTPUT )
mkdir -p $OUTPUTDIR

cp "$MERGELIST" .
ORIGMERGELIST=$( basename "$MERGELIST" )
TMPMERGELIST="$TMPDIR/mergelist.tmp"
TMPMERGEDIR="$TMPDIR/mergedir"
TMPOUTPUT=$( basename "$MERGEOUTPUT" )
TMPOUTPUT=${TMPOUTPUT%.bz2}
TMPLOG=$( basename "$MERGELOG" )
mkdir -p "$TMPMERGEDIR"

while read mergefile ; do
    echo "line: $mergefile"
    
    # check if file exists
    if [ ! -f "$mergefile" ] ; then 
        echo "Error, tried to merge '$mergefile', which doesn't seem to exist, exiting..."
        exit 1
    fi
    
    # copy it to the temp dir
    cp "$mergefile" "$TMPMERGEDIR/"
    SINGLEVBF="$TMPMERGEDIR/`basename $mergefile`"
    
    # un-bzip2 it if needed
    if [[ "${SINGLEVBF##*.}" == "bz2" ]] ; then
        bzip2 -d "$SINGLEVBF"
        SINGLEVBF="${SINGLEVBF%.bz2}"
    fi
    
    # and add it to the tmpdir mergelist
    echo "$SINGLEVBF" >> "$TMPMERGELIST"
    
done < "$ORIGMERGELIST"

# see whats in TMPDIR
echo
ls -lh $TMPDIR
echo

# see whats in our TMPMERGEDIR
echo
ls -ls "$TMPMERGEDIR"
echo

# merge all the tempdir
cd $SIMUSYS
mergeVBF "$TMPMERGELIST" "$TMPOUTPUT" 98765 > "$TMPLOG" 2>&1

# copy merged log to location
rm -rf $MERGELOG
cp $TMPLOG $MERGELOG
echo "LINE MERGELOG $MERGELOG"

# copy tmpmergelog to location
cp "$TMPMERGELIST" `dirname $MERGELIST`/
echo "LINE TMPMERGELIST `dirname $MERGELIST`/`basename $TMPMERGELIST`"

# self-check for problems
if [ ! -f "$TMPOUTPUT" ] ; then
    echo "Error, mergeVBF should have created file '$TMPOUTPUT' by now, something is wrong, exiting..."
    exit 1
fi

# bzip2 if necessary
if [[ "${OUTPUTDATAFILE##*.}" == "bz2" ]] ; then
    bzip2 -z $TMPOUTPUT
    TMPOUTPUT="${TMPOUTPUT}.bz2"
fi

# copy merged datafile to target
rm -rf $MERGEOUTPUT
cp $TMPOUTPUT $MERGEOUTPUT
echo "LINE MERGEDATA $MERGEOUTPUT"


exit 0 

