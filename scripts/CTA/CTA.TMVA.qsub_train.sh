#$ -S /bin/tcsh
#
# script to train cuts/MVAs with TMVA
#
#
# Author: Gernot Maier
#

set RPARA=RUNPARA
set WFILE=OFIL

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

rm -f $RPARA.log
$EVNDISPSYS/bin/trainTMVAforGammaHadronSeparation $RPARA.runparameter > $RPARA.log

# mv weight file into the right place
mv -f $WFILE"_0".class.C $WFILE.class.C
mv -f $WFILE"_0".weights.xml $WFILE.weights.xml

exit

