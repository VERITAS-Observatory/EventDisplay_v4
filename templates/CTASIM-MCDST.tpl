# 
# Template describing a MonteCarlo DST
# Events + telescope-wise parameters + MonteCarlo information
#
#
#
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
#\include cta-event-hillas.inc
\include cta-event-mc.inc
\include cta-dst-hillas.inc

#\include cta-dst-intensity.inc
#
# ======================================================================
# The array info table describing the telescope layout
# ======================================================================
#
\include cta-arrayinfo.tpl
#
# ======================================================================
# The monte-carlo info table describing the telescope layout 
# only needs to be included for montecarlo runs
# ======================================================================
#
\include cta-mcinfo.tpl
#
# ======================================================================
# The analysis info table describing what analysis was performed to  
# generate the event list
# ======================================================================
#
\include cta-analysis.tpl
