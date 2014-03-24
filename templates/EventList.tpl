SIMPLE T
BITPIX = 16
NAXIS = 0
EXTEND = T
#
# ======================================================================
# The main EVENTS extension is made up of possible several included column 
#  definition files:
# ======================================================================
#
XTENSION =   BINTABLE     / Binary table extension
EXTNAME  =   EVENTS       / Extension name
\include cta-run-header.inc
\include cta-event-base.inc
\include cta-event-hillas.inc
\include cta-arrayinfo.tpl
\include cta-gti.tpl
