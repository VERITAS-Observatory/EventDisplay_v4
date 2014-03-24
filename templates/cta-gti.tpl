################################################################################
#
# DATANAME CTA - GTI (good time interval) table
#
# DESCRIPTION gives time interval in which data should be analyzed
#
# CONTENT bintable definition, see ogip/93/003 memo
#
#
# TODO 
#
#
# CHANGES
#
################################################################################
XTENSION = BINTABLE     / Binary table extension
EXTNAME  = GTI          / Extension name
TSTART   = 0            / start time of entire GTI table, in mission time
TSTOP    = 0            / end time of entire GTI table, in mission time
TIMEZERO = 0            / zero time used in standard time units
TIMEUNIT = 's'          / units for the time related keywords
TIMESYS = 'TT      '    / type of time system that is used
TIMEREF = 'LOCAL   '    / reference frame used for times
HDUCLASS = 'OGIP'       / format conforms to OGIP standard
HDUCLAS1 = 'GTI'        / extension contains good time intervals
HDUCLAS2 = 'ALL'        / extension contains all science time
MJDREFI =        51910. / Integer part of MJD of reference time
MJDREFF = '7.428703703703703D-4' / Fractional part of MJD of reference time
ttype#   = START        / start time of interval in MET
tform#   = 1E         
tunit#   = 's'          
ttype#   = STOP         / start time of interval in MET
tform#   = 1E
tunit#   = 's'          