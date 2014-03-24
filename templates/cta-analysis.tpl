################################################################################
#
# DATANAME CTA - Event selection information
#
# DESCRIPTION
#
#   table of what event selection parameters were applied. In principle
#   one could just store a "config" name, but this is more general and
#   more useful if non-standard cuts have been applied
#
# TEMPLATE HEGR-EVTS-ALL
#
# CONTENT header keywords
#
# CHANGES
#
################################################################################
XTENSION = BINTABLE
EXTNAME  = ANALYSIS
ANA_TYPE = unknown         / analysis type (e.g. hillas, m3d, m2d, etc.)
ANA_VER  = unknown         / version of analysis 
ANA_DATE = 1970-01-01      / date analysis was performed
ANA_TIME = 00:00:00        / time analysis was started
CALI_VER = unknown         / version string for the calibration used
CONFIG   = unknown         / analysis config name (if standard)
CLEANING = 'tail:05,10'    / string describing image cleaning applied 
ttype# = PARAM             / parameter that was cut
tform# = 128A              
ttype# = UNIT              / unit for parameter that was cut
tform# = 30A              
ttype# = CUT_MIN           / minimum cut value
tform# = 1D
ttype# = CUT_MAX           / maximum cut value
tform# = 1D
ttype# = N_EVTS            / number of events passing the cut (if known) 
tform# = 1J
COMMENT end of analysis table
