#!/bin/tcsh
#
# script to optimize cuts with TMVA
#
# Revision $Id: qsub_optimizeBoxCuts_TMVA.sh,v 1.1.2.1 2011/04/11 16:11:21 gmaier Exp $
#
# Author: Gernot Maier
#

set RPARA=RUNPARA
set WFILE=OFIL

# set the right observatory (environmental variables)
source $EVNDISPSYS/setObservatory.tcsh CTA

rm -f $RPARA.log
$EVNDISPSYS/bin/makeOptimizeBoxCutsTMVA $RPARA.runparameter > $RPARA.log

# mv weight file into the right place
mv -f $WFILE"_0".class.C $WFILE.class.C
mv -f $WFILE"_0".weights.xml $WFILE.weights.xml

exit

