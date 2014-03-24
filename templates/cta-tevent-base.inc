################################################################################
#
# DATANAME CTA - base telescope wise data table
#
# DESCRIPTION 
#
# This is an include file - the extension must be defined before
# including this.
#
# CONTENT bintable definition
#
# CHANGES
#  - 1.1: added NULL value for TIMESLICE (in case there is no timing)
################################################################################
EVTVER = 1.1       / event list version number
ttype# = EVENT_ID  / event number
tform# = 1V
ttype# = TEL_ID    / telescope ID
tform# = 1I
ttype# = TIMESLICE / timeslice number
tform# = 1I
tnull# = 0
ttype# = TEL_IMPACT / impact parameter of shower w.r.t. a tel
tform# = 1E
tunit# = m
ttype# = TEL_INTENS / integrated intensity of image (P.E.)
tform# = 1E
tunit# = p.e.
ttype# = TEL_HIL_COGX / x coord of first moment in camera coords
tform# = 1E
tunit# = deg
ttype# = TEL_HIL_COGY / y coord of first moment in camera coords
tform# = 1E
tunit# = deg
ttype# = TEL_HIL_WIDTH / second moment of shower image (deg)
tform# = 1E
tunit# = deg
ttype# = TEL_HIL_LENGTH / second moment of shower image (deg)
tform# = 1E
tunit# = deg
ttype# = TEL_HIL_PHI / rotation angle of shower image wrt x-axis (deg)
tform# = 1E
tunit# = deg
ttype# = TEL_HIL_SKEW / skewness (3rd order moments)
tform# = 1E
tunit# = deg
ttype# = TEL_HIL_KUR / kurtosis (3rd order moments)
tform# = 1E
tunit# = deg