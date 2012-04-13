#!/bin/tcsh
#
# script to analyse files 
#
# Revision $Id: qsub_evndisp.sh,v 1.1.2.1.4.2.2.1.4.1.4.2.2.6.2.9.2.7.4.1.2.1 2011/04/06 11:46:33 gmaier Exp $
#
# Author: Gernot Maier
#
set RUN=RRRRR
set PED=PEEED

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh VERITAS

# output data files are written to this directory
set ODIR=$VERITAS_USER_DATA_DIR"/analysis/EVD400/"
mkdir -p $ODIR
# output log files are written to this directory
set LDIR=$VERITAS_USER_LOG_DIR"/analysis/EVD400/"
mkdir -p $LDIR

# eventdisplay reconstruction parameter
set ACUTS="EVNDISP.reconstruction.runparameter"

#########################################
# directory with executable
cd $EVNDISPSYS/bin/

# pedestal
if( $PED == "1" ) then
    rm -f $LDIR/$RUN.ped.log
    ./evndisp -runnumber=$RUN -runmode=1  > $LDIR/$RUN.ped.log
endif

set OPT=" "
# pointing from db (T-Point corrected)
#set OPT="$OPT -teltoana=234 "
# pointing from db using T-point correction from 2007-11-05
#set OPT="$OPT -useDBtracking -useTCorrectionfrom "2007-11-05""
# pointing from pointing monitor (text file)
#set OPT="$OPT -pointingmonitortxt /raid/pevray/maierg/veritas/VPM/results/"
# pointing from pointing monitor (DB)
# set OPT="$OPT -usedbvpm "
# OFF data run
#set OPT="$OPT -raoffset=6.25"
# use calib.dat
# set OPT="$OPT -calibrationfile calib.dat"

# run eventdisplay
rm -f $LDIR/$RUN.log
./evndisp -runnumber=$RUN -reconstructionparameter $ACUTS -outputfile $ODIR/$RUN.root $OPT > $LDIR/$RUN.log

# sleep for 20 s 
sleep 20

exit

