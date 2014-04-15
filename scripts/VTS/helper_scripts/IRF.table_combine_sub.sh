#!/bin/bash
# combine tables
# Author: Gernot Maier

# parameters replaced by parent script using sed
FLIST=TABLELIST
OFILE=OUTPUTFILE
ODIR=OUTPUTDIR

# combine the tables
$EVNDISPSYS/bin/combineLookupTables $FLIST $ODIR/$OFILE.root &> $ODIR/$OFILE.log 

exit
