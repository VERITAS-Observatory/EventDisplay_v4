#!/bin/tcsh
#
# Running effective for CTA
#
# Revision $Id$
#
# Author: Gernot Maier
#
###################################################################################
# VALUES SET BY MOTHER SCRIPT
###################################################################################
# input file list 
set MSCFIL=IIIIFIL
# effective area file (output)
set OFIL=TTTTFIL
###################################################################################

###################################################################################
# run effective area code
$EVNDISPSYS/bin/makeEffectiveArea $MSCFIL $OFIL.root > $OFIL.log
###################################################################################

exit
