#$ -S /bin/tcsh
#
# script to analyse VTS raw files (VBF) with eventdisplay
#
# Author: Gernot Maier
#

#####################################
# parameters set by parent script
set RUN=RRRRR
set CALIB=PEEED
set VPM=VVPM
set ODIR=OODIR
set LDIR="$ODIR"
mkdir -p $ODIR
mkdir -p $LDIR

#####################################
# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh VERITAS


#####################################
# eventdisplay reconstruction parameter
set ACUTS="EVNDISP.reconstruction.runparameter"

#########################################
# directory with executable
cd $EVNDISPSYS/bin/

#########################################
# pedestal calculation
if( $CALIB == "1" || $CALIB == "2" ) then
    rm -f $LDIR/$RUN.ped.log
    ./evndisp -runnumber=$RUN -runmode=1  > $LDIR/$RUN.ped.log
endif

#########################################
# different command line options
set OPT=" "
#read gain and toff from VOFFLINE DB
set OPT="$OPT -readCalibDB "
# none of the following command line options is needed for the standard analysis!
#read gain and toff from VOFFLINE DB requiring a special version of analysis 
# set OPT = "-readCalibDB version_number"
# warning this version must already exist in the DB
#read gain and toff from VOFFLINE DB and save the result of the reading in the directory where the calib file should be (it won't erase what is already there)
# set OPT = "-readandsavecalibdb"
# quick look option (has no effect if readCalibDB or equivalent is set)
# to be set when not caring about the calibration information
# if no gain can be read from your laser File, they will be set to 1
# if no toffset can be read from your laser File, tey will be set to 0
#set OPT="$OPT -nocalibnoproblem"
# if this option is NOT set, the analysis break if there is problem reading the gain and toffset files


#########################################
# average tzero calculation
if( $CALIB == "1" || $CALIB == "3" ) then
    rm -f $LDIR/$RUN.tzero.log
    ./evndisp -runnumber=$RUN -runmode=7 $OPT > $LDIR/$RUN.tzero.log
endif

#########################################
# different command line options for pointing and calibration

# pointing from db (T-Point corrected)
# set OPT="$OPT -teltoana=134 "
# pointing from db using T-point correction from 2007-11-05
#set OPT="$OPT -useDBtracking -useTCorrectionfrom "2007-11-05""
# pointing from pointing monitor (DB)
if( $VPM == "1" ) then
   set OPT="$OPT -usedbvpm "
endif
# OFF data run
#set OPT="$OPT -raoffset=6.25"
# use text file for calibration information
# set OPT="$OPT -calibrationfile calibrationlist.dat"
# double passed correction
#set OPT="$OPT -nodp2005"

#########################################
# run eventdisplay
rm -f $LDIR/$RUN.log
./evndisp -runnumber=$RUN -reconstructionparameter $ACUTS -outputfile $ODIR/$RUN.root $OPT > $LDIR/$RUN.log

#########################################
# sleep for 20 s 
sleep 20

exit
