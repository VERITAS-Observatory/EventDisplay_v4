#!/bin/bash
#
# script to run evndisp for grisu simulations on one of the cluster nodes (VBF)
#
# Gernot Maier
#
##############################################################################################
# variables set by parent script
##############################################################################################
INPUTFILE=INPUTFILEINPUTFILE
RUNNUM=RUNRUNRUN
OUTPUTDIR=OUTPUTDIROUTPUTDIR
NOISE=NOISENOISE
NOISEFILE=NOISEFILENOISEFILE
ARRAYCONFIG=CONFIGCONFIG

echo "ARRAYCONFIG='$ARRAYCONFIG'"
mkdir -p $OUTPUTDIR

##############################################################################################
# input files
# (observe that these might need some adjustments)
##############################################################################################

source $EVNDISPSYS/setObservatory.sh VTS

# temp directory
if [[ -n $TMPDIR ]]; then
    TEMPDIR=$TMPDIR/$RUN
else
    TEMPDIR="$VERITAS_USER_DATA_DIR/TMPDIR"
fi
mkdir -p $TEMPDIR
cd $TEMPDIR

##############################################################################################
# directory with executable
#cd $EVNDISPSYS/scripts/VTS/
##############################################################################################
# detector configuration and cuts
# telescopes
TTA="1234"
#TTA="234"
# ACUT="EVNDISP.reconstruction.CORR.runparameter"
ACUT="EVNDISP.reconstruction.runparameter"
# dead channel definitions
DEAD="EVNDISP.validchannels.dat"
# default pedestal level
PEDLEV="16."
##############################################################################################

echo "INPUTFILE: $INPUTFILE"
if [[ ! -f $INPUTFILE ]] ; then
    echo "Error, file \$INPUTFILE=$INPUTFILE doesn't exist, exiting..."
    exit 1
fi

ONCLUSTER=false
# $TMPDIR exists, and we are on a cluster
if [[ -n $TMPDIR ]] ; then 
    ONCLUSTER=true
fi
echo "ONCLUSTER=$ONCLUSTER"

# if on a cluster, copy input file to temporary directory
if [[ $ONCLUSTER ]] ; then
    # on a cluster (like at desy), copying the input datafile to the temporary directory 
    # (thats close to the processor that does the job) can greatly reduce network traffic
    cp $INPUTFILE $TEMPDIR/
    SIMFILE=$TEMPDIR/`basename $INPUTFILE`
    if [[ ! -f $SIMFILE ]] ; then
        echo "Error, copied temporary file \$SIMFILE='$SIMFILE' doesn't exist, (copied from '$INPUTFILE' ), exiting..."
        exit 1
    fi
else
    SIMFILE="$INPUTFILE"
fi
echo "A: SIMFILE=$SIMFILE"

# if simulation file is bzip2'd, extract it
if [[ "$SIMFILE" == *.vbf.bz2 ]] ; then
    echo " SIMFILE is bzipped, extracting it now"
    bzip2 -dk $SIMFILE
    # convert filename.vbf.bz2 to filename.vbf
    SIMFILE=$( echo $SIMFILE | sed 's/\(.*\)\..*/\1/' )
fi

# now have our inputfile in the right place, and its also a .vbf file
if [[ ! -f $SIMFILE ]] ; then
    echo "Error, SIMFILE='$SIMFILE' doesn't exist, exiting..."
    exit 1
fi


##########################################################################################
# eventdisplay run options
##########################################################################################

##### pedestal options #####
PEDOPT=( -pedestalfile $NOISEFILE -pedestalseed=$RRR -pedestalDefaultPedestal=$PEDLEV -pedestalnoiselevel=$NOISE )
# call this later via ${PEDOPT[@]} with no quotes to feed all the arguments properly to bin/evndisp

##### MC options #####
# restrict average tzero calculation to inner part of the camera
# (note that we are simulating gamma rays in general)
# MCOPT=( -sourcetype=2 -camera=$ARRAYCONFIG -averagetzerofiducialradius=0.60 )
# default options
MCOPT=( -sourcetype=2 -camera=$ARRAYCONFIG )
# call this later via ${MCOPT[@]} with no quotes to feed all the arguments properly to bin/evndisp

echo "RUNNUMBER $RUNNUM"
EVNDISPOUTNAME="$RUNNUM.sim"
# this will create:
#   $OUTPUTDIR/$EVNDISPOUTNAME.tzero.log
#   $OUTPUTDIR/$EVNDISPOUTNAME.evndisp.log
#   $OUTPUTDIR/$EVNDISPOUTNAME.evndisp.root
echo "EVNDISP outputfile root file written to $OUTPUTDIR/$EVNDISPOUTNAME.evndisp.root"
echo "EVNDISP log file written to $OUTPUTDIR/$EVNDISPOUTNAME.evndisp.log"

##########################################################################################
# calculating tzeros
##########################################################################################
echo "CALCULATING AVERAGE TZEROS FOR RUN $RUNNUM"
rm -f $ODIR/$RUNNUM.tzero.log
$EVNDISPSYS/bin/evndisp               \
    -sourcetype=2                     \
    -sourcefile $SIMFILE              \
    -teltoana=$TTA                    \
    -runmode=7                        \
    -runnumber=$RUNNUM                \
    -deadchannelfile $DEAD            \
    -arraycuts $ACUT                  \
    -calibrationsumwindow=20          \
    -calibrationsumfirst=0            \
    -donotusedbinfo                   \
    -calibrationnevents==100000       \
    ${PEDOPT[@]}                      \
    -calibrationdirectory $OUTPUTDIR/ \
    -lowgaincalibrationfile NOFILE    \
    > $OUTPUTDIR/$EVNDISPOUTNAME.tzero.log

echo "RUN$RUNNUM TZEROLOG $OUTPUTDIR/$EVNDISPOUTNAME.tzero.log"


# if we're on the cluster, write the output *.evndisp.root file 
# to local directory first, to reduce network traffic
# then copy it to $OUTPUTDIR at the end of the script
if [[ $ONCLUSTER ]] ; then
    FINALOUTNAME=$TEMPDIR/$EVNDISPOUTNAME.evndisp.root
else
    FINALOUTNAME=$OUTPUTDIR/$EVNDISPOUTNAME.evndisp.root
fi
##########################################################################################
# run eventdisplay 
##########################################################################################
$EVNDISPSYS/bin/evndisp                 \
    -runnumber=$RUNNUM                  \
    -writenomctree                      \
    -sourcefile $SIMFILE                \
    -deadchannelfile $DEAD              \
    -arraycuts $ACUT                    \
    -outputfile $FINALOUTNAME           \
    -teltoana=$TTA                      \
    ${MCOPT[@]}                         \
    ${PEDOPT[@]}                        \
    -calibrationdirectory $OUTPUTDIR/   \
    -lowgaincalibrationfile NOFILE      \
    > $OUTPUTDIR/$EVNDISPOUTNAME.evndisp.log

echo "RUN$RUNNUM EVNLOG $OUTPUTDIR/$EVNDISPOUTNAME.evndisp.log"
##########################################################################################

sleep 10s

# if on cluster, move evndisp.root file from $TEMPDIR to $OUTPUTDIR
if [[ $ONCLUSTER ]] ; then
    mv -fv $TEMPDIR/$EVNDISPOUTNAME.evndisp.root $OUTPUTDIR/
fi
echo "RUN$RUNNUM DATAOUT $OUTPUTDIR/$EVNDISPOUTNAME.evndisp.root"

exit 0
