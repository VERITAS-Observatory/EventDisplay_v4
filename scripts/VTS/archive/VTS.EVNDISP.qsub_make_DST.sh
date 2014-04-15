#$ -S /bin/tcsh
#
# script to make DSTs
#
# Author: Gernot Maier
# edited by G Hughes to make SW a variable
#
set RUN=RRRRR
set PED=PEEED
set SUM=SWSW

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh VERITAS

# output data files are written to this directory
set ODIR=$VERITAS_USER_DATA_DIR"/analysis/EVD400_DST/"$SUM"/"
mkdir -p $ODIR
# output log files are written to this directory
set LDIR=$VERITAS_USER_LOG_DIR"/analysis/EVD400_DST/"$SUM"/"
mkdir -p $LDIR


if[ -e $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.reconstruction.SW"$SUM"_noDoublePass.runparameter ]
then
 rm $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.reconstruction.SW"$SUM"_noDoublePass.runparameter
fi
cat $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.reconstruction.LOWGAIN.runparameter | sed s/"XX"/"$SUM"/g > $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.reconstruction.SW"$SUM"_noDoublePass.runparameter


# eventdisplay reconstruction parameter
set ACUTS="EVNDISP.reconstruction.SW"$SUM"_noDoublePass.runparameter"

#########################################
# directory with executable
cd $EVNDISPSYS/bin/

# pedestal
if( $PED == "1" ) then
    rm -f $LDIR/$RUN.ped.log
    ./evndisp -runnumber=$RUN -runmode=1  > $LDIR/$RUN.ped.log
endif

#tzero
if( $PED == "1" ) then
    rm -f $LDIR/$RUN.tzero.log
    ./evndisp -runnumber=$RUN -runmode=7 -nocalibnoproblem > $LDIR/$RUN.tzero.log
endif

set OPT=" "

# run eventdisplay
rm -f $LDIR/$RUN.log
./evndisp -runnumber=$RUN -runmode=4 -nevents=5000  -nocalibnoproblem -reconstructionparameter $ACUTS -dstfile $TMPDIR/$RUN.DST.root $OPT > $LDIR/$RUN.DST.log

mv -v -f $TMPDIR/$RUN.DST.root $ODIR/$RUN.DST.root

rm $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.reconstruction.SW"$SUM"_noDoublePass.runparameter

# sleep for 20 s 
sleep 1

exit

