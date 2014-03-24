################################################################################
#
# DATANAME CTA - RUNHEADER
#
# DESCRIPTION
# contains all run information as header keywords. An extension must
# be defined before including this.
#
# TEMPLATE HEGR-EVTS-ALL
#
# CONTENT header keywords
#
# CHANGES
#
################################################################################
CREATOR  =   unknown        / program that created the data in this file
TELESCOP =   CTA            / mission name
OBS_ID   =   0              / observation number
DATE_OBS =   1970-01-01     / human-readable start date of obs yy-mm-dd
TIME_OBS =   00:00:00       / human-readable start time of obs hh:mm:ss 
DATE_END =   1970-01-01     / human-readable end date of obs yy-mm-dd
TIME_END =   00:00:00       / human-readable end time of obs hh:mm:ss 
# ========================================================================
TSTART   = 0                / MET of observation start time
TSTOP    = 0                / MET of observation stop time
MJDREFI  = 51910            / int part of reference MJD for times
MJDREFF  = 7.4287037e-4     / fractional part of MJDREF
TIMEUNIT = s                / time unit is seconds since MET start
TIMESYS  = TT               / Time system (TT=terrestrial time)
TIMEREF  = local            / local time reference
TASSIGN  = Namibia          / where time reference is assigned
TELAPSE  = 0                / difference of start and end times
ONTIME   = 0                / total good time (including deadtime)
LIVETIME = 0                / total live time of run (dead=ONTIME/LIVETIME)
DEADC    = 0                / average deadtime fraction
# ========================================================================
OBJECT   = unknown          / observed object
RA_OBJ   = 0                / RA of target object (deg) 
DEC_OBJ  = 0                / Dec of target object (deg)
RA_PNT   = 0                / nominal pointing position RA (deg)
DEC_PNT  = 0                / nominal pointing position Dec (deg)
ALT_PNT  = 0                / mean altitude of run (deg)
AZ_PNT   = 0                / mean azimuth of run (deg)
RADECSYS = FK5              / equatorial system type 
EQUINOX  = 2000             / base equinox
CONV_DEP = 0                / convergence depth of telescopes, 0 for parallel
CONV_RA  = 0                / convergence position RA (deg)
CONV_DEC = 0                / convergence position Dec (deg)
OBS_MODE = wobble           / observation mode (wobble,scan,on,off)
# ========================================================================
N_TELS   = 0                / number of telescopes participating
TELLIST  = 0                / comma-separated list of telescope IDs in array
GEOLAT   = 0                / latitude of observatory (deg)
GEOLON   = 0                / longitude of observatory (deg)
ALTITUDE = 0                / altitude of observatory (km)
# ========================================================================
EUNIT    = TeV              / energy unit 