#!/bin/tcsh
#
# combine tables
#
# Revision $Id: qsub_combineTablesFiles.sh,v 1.1.2.1 2011/04/18 07:48:45 gmaier Exp $
#
# Author: Gernot Maier
#

set ILIST=LLIISSTT
set OFIL=OOFFIILLEE

source $EVNDISPSYS/setObservatory.tcsh VERITAS

# this is where the executable can be found
cd $EVNDISPSYS/VTS/scripts

# combine the tables

$EVNDISPSYS/bin/combineTables $ILIST $OFIL.root > $OFIL.log 

exit
