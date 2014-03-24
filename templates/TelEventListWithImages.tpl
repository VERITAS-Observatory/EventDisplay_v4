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
EXTNAME  =   TEVENTS       / Extension name
\include cta-run-header.inc
\include cta-tevent-base.inc
\include cta-tevent-intensity.inc
\include cta-arrayinfo.tpl
\include cta-gti.tpl
