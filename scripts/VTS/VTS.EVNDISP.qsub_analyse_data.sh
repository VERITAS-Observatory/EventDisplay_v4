#$ -S /bin/tcsh
#
# script to analyse files 
#
# Author: Gernot Maier
#
set RUN=RRRRR
set CALIB=PEEED

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh VERITAS

# output data files are written to this directory
set ODIR=$VERITAS_USER_DATA_DIR"/analysis/EVD400-d20130320_rDB/"
mkdir -p $ODIR
# output log files are written to this directory
set LDIR=$VERITAS_USER_LOG_DIR"/analysis/EVD400-d20130320_rDB/"
mkdir -p $LDIR

# eventdisplay reconstruction parameter
set ACUTS="EVNDISP.reconstruction.runparameter"
# set ACUTS="EVNDISP.reconstruction.CORR.runparameter"

#########################################
# directory with executable
cd $EVNDISPSYS/bin/

# pedestal calculation
if( $CALIB == "1" || $CALIB == "2" ) then
    rm -f $LDIR/$RUN.ped.log
    ./evndisp -runnumber=$RUN -runmode=1  > $LDIR/$RUN.ped.log
endif

set OPT=" "
#read gain and toff from VOFFLINE DB
set OPT="$OPT -readCalibDB "
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


# average tzero calculation
if( $CALIB == "1" || $CALIB == "3" ) then
    rm -f $LDIR/$RUN.tzero.log
    ./evndisp -runnumber=$RUN -runmode=7 $OPT > $LDIR/$RUN.tzero.log
endif

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
# double passed correction
#set OPT="$OPT -nodp2005"



# run eventdisplay
rm -f $LDIR/$RUN.log
./evndisp -runnumber=$RUN -reconstructionparameter $ACUTS -outputfile $ODIR/$RUN.root $OPT > $LDIR/$RUN.log

# sleep for 20 s 
sleep 20

exit

