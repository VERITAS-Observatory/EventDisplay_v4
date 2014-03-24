#
# Template for a real-data DST 
# (events + telescope parameters)
#
#
SIMPLE T
BITPIX = 16
EXTEND = T
XTENSION =   BINTABLE     / Binary table extension
EXTNAME  =   SPECRESP     / Extension name
TELESCOP = 'unknown'
INSTRUME = 'unknown'
HDUCLASS = OGIP           / organization of definition
HDUDOC   = CAL/GEN/92-019 / document describing format
HDUCLAS1 = RESPONSE
HDUCLAS2 = SPECRESP
HDUVERS = 1.0.0
FILTER =                  / relic from X-ray analysis (required)
ttype# = ENERG_LO        / Lower energy bound
tform# = 1E
tunit# = TeV
ttype# = ENERG_HI        / Upper energy bound
tform# = 1E
tunit# = TeV
ttype# = SPECRESP         / Effective Area
tform# = 1E
tunit# = m^2
