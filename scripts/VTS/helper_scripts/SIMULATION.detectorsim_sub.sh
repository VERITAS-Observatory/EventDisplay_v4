#!/bin/bash
#$ -l os=sl6
#$ -l h_cpu=00:29:00
#$ -l h_vmem=4G
#$ -l tmpdir_size=30G
#$ -j y
#$ -js 20
#

# set observatory environmental variables
source $EVNDISPSYS/setObservatory.sh VTS
source $EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_functions.sh

SIMTRACKFILE=TRACKTRACKTRACK
if [ ! -f $SIMTRACKFILE ] ; then
    echo "error, SIMTRACKFILE '$SIMTRACKFILE' doesn't exist, exiting..."
    exit 1
fi

# get to the temp dir
cp $SIMTRACKFILE $TMPDIR/
cd $TMPDIR
SIMTRACKFILE="$TMPDIR/`basename $SIMTRACKFILE`"

LINENUMBER=$SGE_TASK_ID

SIMTRACKLINE=$( sed $LINENUMBER'q;d' $SIMTRACKFILE )
echo "Loading line: '$SIMTRACKLINE'"
echo "$SIMTRACKLINE" | tr '+' '\n' | awk '{ printf "  %s\n", $0 }'


# load job variables
INPUTCORSIKA=$(   getTagArgFromTagline "inputcorsikafile" "$SIMTRACKLINE" )
CRUN=$(           getTagArgFromTagline "corsikajobiter"   "$SIMTRACKLINE" )
OUTPUTDATAFILE=$( getTagArgFromTagline "outputdatafile"   "$SIMTRACKLINE" )
OUTPUTLOGFILE=$(  getTagArgFromTagline "outputlogfile"    "$SIMTRACKLINE" )
ARRAYCONFIG=$(    getTagArgFromTagline "arrayconfig"      "$SIMTRACKLINE" )
EXTFILENAME=$(    getTagArgFromTagline "extfilename"      "$SIMTRACKLINE" )
ARRAYEPOCH=$(     getTagArgFromTagline "arrayepoch"       "$SIMTRACKLINE" )
FASTDEVRUN=$(     getTagArgFromTagline "fastdevrun"       "$SIMTRACKLINE" )
DISPERSAL=$(      getTagArgFromTagline "dispersal"        "$SIMTRACKLINE" )
PARTICLE=$(       getTagArgFromTagline "particle"         "$SIMTRACKLINE" )
SIMTYPE=$(        getTagArgFromTagline "simtype"          "$SIMTRACKLINE" )
SAVELOG=$(        getTagArgFromTagline "savelog"          "$SIMTRACKLINE" )
WOBBLE=$(         getTagArgFromTagline "wobble"           "$SIMTRACKLINE" )
echo
SIMBASE=$( echo "$SIMTYPE" | tr '_' ' ' | awk '{ print $1 }' | sed -e 's|^ *||' -e 's| *$||' | grep -oP "[A-Za-z]+" )

if [[ "$FASTDEVRUN" == "yes" ]] ; then
    echo "Warning, using FASTDEVRUN, will not process all events"
fi

rm -rf $OUTPUTDATAFILE
rm -rf $OUTPUTLOGFILE

OUTPUTLOGDIR=$( dirname $OUTPUTLOGFILE )

# start the detsim
if [[ "$SIMBASE" == "grisu" ]] ; then
    echo
    echo "Starting detector simulation with GrISU"
    
    # get array config file
    cp -fv $VERITAS_EVNDISP_AUX_DIR/DetectorGeometry/$ARRAYCONFIG .
    
    # set up pilot file
    PILOT="DAT${CRUN}.pilot"
    touch $PILOT
    
    # setup grisu files
    FIFOFILE="DAT${CRUN}.grisu.fifo"
    GLOG="DAT${CRUN}.log"
    OUTPUTVBF=$( basename $OUTPUTDATAFILE )
    OUTPUTVBF=${OUTPUTVBF%.bz2}
    
    # fill the pilot file
    echo "* ARRAY $TMPDIR/$ARRAYCONFIG" >> $PILOT
    echo "* CHERP $TMPDIR/$FIFOFILE"    >> $PILOT
    echo "* LOGBK $TMPDIR/$GLOG"        >> $PILOT
    echo "* VBFOF $TMPDIR/$OUTPUTVBF"   >> $PILOT

    if   [[ "$ARRAYEPOCH" == "4" ]] ; then echo "* VBFST 2009-07-22 02:00:00.000000000  GPS"  >> $PILOT
    elif [[ "$ARRAYEPOCH" == "5" ]] ; then echo "* VBFST 2009-09-10 02:00:00.000000000  GPS"  >> $PILOT
    elif [[ "$ARRAYEPOCH" == "6" ]] ; then echo "* VBFST 2012-10-10 02:00:00.000000000  GPS"  >> $PILOT
    else ( echo "Error, invalid ARRAYEPOCH '$ARRAYEPOCH', exiting..." ; exit 1 )
    fi

    SEED=$(( CRUN * 10 ))
    echo "* VBFTM 200.0 1 1"                      >> $PILOT
    echo "* VBFPR $USER $CRUN 1275 1 1" >> $PILOT
    echo "* RANDM -$SEED"                         >> $PILOT
    echo "* NBRPR 0 1"                            >> $PILOT

    # check needed arguments
    if [[ ! "$DISPERSAL" =~ (diffuse|ring) ]] ; then
        echo "Error, Unrecognized DISPERSAL argument '$DISPERSAL', exiting..." ; exit 1
    fi
    if [[ ! "$PARTICLE" =~ (gamma|electron|proton|helium) ]] ; then
        echo "Error, urecognized PARTICLE argument '$PARTICLE', exiting..." ; exit 1
    fi
    
    # fill source info
    NREPEATS=0
    DIFFUSERADIUS=0.0
    if   [[ "$PARTICLE" == "gamma"    ]] ; then
        NREPEATS=8
        DIFFUSERADIUS=2.75
    elif [[ "$PARTICLE" == "electron" ]] ; then
        NREPEATS=10
        DIFFUSERADIUS=2.00
    elif [[ "$PARTICLE" == "proton" ]] ; then
        NREPEATS=10
        DIFFUSERADIUS=4.0
    elif [[ "$PARTICLE" == "helium" ]] ; then
        NREPEATS=15
        DIFFUSERADIUS=4.0
    fi
    if   [[ "$DISPERSAL" == "diffuse" ]] ; then echo "* SOURC 0.0 0.0 $DIFFUSERADIUS 31.675" >> $PILOT
    elif [[ "$DISPERSAL" == "ring"    ]] ; then echo "* SOURC 0.0 $WOBBLE 0.0 31.675"        >> $PILOT
    fi
    
    if   [ "$FASTDEVRUN"   == "yes"     ] ; then 
        NREPEATS=2
        echo "FASTDEVRUN: NREPEATS=$NREPEATS"
    fi
    echo "* NBREV 0 $NREPEATS" >> $PILOT
    
    # noise info (no noise)
    echo "* NOISE 0. 0. 0." >> $PILOT
    echo "* NUMPE 0"        >> $PILOT
    echo "* FRECP 0"        >> $PILOT
    echo "* NOPIX 0"        >> $PILOT
    echo "* USEOC 0"        >> $PILOT
    echo "* GRIDF 1 15 15"  >> $PILOT
    echo "* GRIDP 1 19 19"  >> $PILOT
    
    # show the pilot file
    if [[ "$SAVELOG" == "dosave" ]] ; then
        cp -f $TMPDIR/$PILOT $OUTPUTLOGDIR/$PILOT
        echo "PILOT $OUTPUTLOGDIR/$PILOT"
    fi
    echo "PILOT FILE $PILOT :"
    cat $PILOT | awk '{ printf "PILOT:%s\n", $0 }'
    echo

    # get corsika file
    cp $INPUTCORSIKA .
    CORSIKAFILE=$( basename $INPUTCORSIKA )
    bzip2 -d $CORSIKAFILE
    CORSIKAFILE=${CORSIKAFILE%.bz2}
    
    # set up extra options
    COROPTS=()
    if [ "$FASTDEVRUN" == "yes" ] ; then
        COROPTS+=( -nevents 150 )
        echo "Warning, corsikaIOreader -nevents 150"
    fi

    # run corsikaIOreader
    echo "corsikaIOreader beginning!"
    IOREADLOG="DAT${CRUN}.corsikaIOreader.log"
    cd $SIMUSYS
    ./corsikaIOreader                              \
        -cors $TMPDIR/DAT$CRUN.telescope           \
        -abs "$EXTFILENAME"                        \
        -grisu "$TMPDIR/$FIFOFILE"                 \
        -shorthisto $TMPDIR/DAT${CRUN}.ioread.root \
        -queff 0.5                                 \
        -seed "$CRUN"                              \
        -cfg "$TMPDIR/$ARRAYCONFIG"                \
        ${COROPTS[@]}                              \
        > $TMPDIR/$IOREADLOG
    echo "corsikaIOreader done!"
    cd $TMPDIR
    
    # copy the corsikaIOreader logfile so the user can read it
    if [[ "$SAVELOG" == "dosave" ]] ; then
        cp -f $TMPDIR/$IOREADLOG $OUTPUTLOGDIR/$IOREADLOG
        echo "IOREADLOG $OUTPUTLOGDIR/$IOREADLOG"
    fi
    echo
    
    # run grisu
    echo "grisudet beginning!"
    GRISULOG=$( basename $OUTPUTLOGFILE )
    $GRISUSYS/grisudet $TMPDIR/$PILOT > $TMPDIR/$GRISULOG
    echo "grisudet done!"
    
    # copy the grisu log file so the user can read it
    if [[ "$SAVELOG" == "dosave" ]] ; then
        cp $TMPDIR/$GRISULOG $OUTPUTLOGDIR/$GRISULOG
        echo "DETSIMLOG $OUTPUTLOGDIR/$GRISULOG"
    fi
    
    if [ ! -f "$TMPDIR/$OUTPUTVBF" ] ; then
        echo "error, output file '$TMPDIR/$OUTPUTVBF' is supposed to exist at this point, but doesn't. exiting..."
        exit 1
    fi
    
    # if output datafile should be bzipped, do so
    if [[ "${OUTPUTDATAFILE##*.}" == "bz2" ]] ; then
        bzip2 -z $TMPDIR/$OUTPUTVBF
        OUTPUTVBF="${OUTPUTVBF}.bz2"
    fi
    
    # move datafile to output directory
    mv -f $TMPDIR/$OUTPUTVBF $OUTPUTDATAFILE
    if [[ ! -f "$OUTPUTDATAFILE" ]] ; then
        echo "Error, failed to properly move file $TMPDIR/$OUTPUTVBF to $OUTPUTDATAFILE"
        exit 1
    fi
    
    # display info
    echo "PWD $PWD"
    ls -lh
    echo
    
    
    
elif [[ "$SIMBASE" == "care" ]] ; then
    echo "Using detector simulatior CARE..."

fi

