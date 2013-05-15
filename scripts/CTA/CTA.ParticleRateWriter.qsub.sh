#$ -S /bin/tcsh
#
# script to write particle rate files
#
# Author: Gernot Maier
#
#######################################################################

set AXRRAY=ARRAY
# directory with effective areas (and directory were files are written to)
set DXDIR=DDIR
set RECID=RRRR
# should be either onSource or cone10
set OXFFSET=OFFSET

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

cd $DXDIR

set LLOG=$DXDIR/ParticleNumbers.$AXRRAY.log
rm -f $LLOG

cd $EVNDISPSYS/
./bin/writeParticleRateFilesFromEffectiveAreas  $AXRRAY $OXFFSET $RECID $DXDIR >! $LLOG 

############################################################################

exit
