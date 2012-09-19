#!/bin/tcsh
#
# combine tables
#
# Author: Gernot Maier
#

set ILIST=LLIISSTT
set OFIL=OOFFIILLEE

source $EVNDISPSYS/setObservatory.tcsh VERITAS

# combine the tables
$EVNDISPSYS/bin/combineLookupTables $ILIST $OFIL.root > $OFIL.log 

exit
