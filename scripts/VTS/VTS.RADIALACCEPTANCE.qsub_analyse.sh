#$ -S /bin/tcsh
#
# analyse effective areas
#
# Author: Gernot Maier
#

cd $EVNDISPSYS
source ./setObservatory.tcsh VTS

###################################################################
# parameters set by parent scripts
###################################################################
# input directory
set DDIR="DDDD"
# output file 
set OFIL=OOOO
# run list
set LLIS=LLLL
# cuts
set CUTS=CCCC

###################################################################
# analyse effective areas
rm -f $OFIL.log
$EVNDISPSYS/bin/makeRadialAcceptance -s $LLIS -c $CUTS -d $DDIR -o $OFIL.root > $OFIL.log

exit
