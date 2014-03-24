################################################################################
#
# DATANAME CTA - base shower table
#
# DESCRIPTION table definition for basic shower parameters (temporal,
# spatial, energy, and trigger information). Nothing analysis-specific
#
# This is an include file - the extension must be defined before
# including this.
#
# CONTENT bintable definition
#
# CHANGES
#
# - 1.3 : removed ALT_PNT, and AZ_PNT (should be separate table), added OBS_ID
#
################################################################################
EVTVER = 1.3       / event list version number
ttype# = EVENT_ID  / event number within the observation
tform# = 1V
ttype# = OBS_ID    / observation id number to cross reference an event to an obs.
tform# = 1V
ttype# = FLAGS      / event type code
tform# = 8x
tunit# = bits
ttype# = TIME     / Timestamp of event from start of run
tform# = 1D        
tunit# = s
ttype# = MULTIP	  / Multiplicity of event (number of tels)
tform# = 1I
ttype# = TELMASK  / bit mask representing telescopes participating in event
tform# = 1X	  / automatically resized to N-tels when written
ttype# = RA	  / RA of event
tform# = 1E
tunit# = deg
ttype# = DEC	  / DEC of event
tform# = 1E
tunit# = deg
ttype# = DIR_ERR  / direction reconstruction error
tform# = 1E
tunit# = deg
tnull# = 0        / NULL value if value isn't known
ttype# = DETX	  / coordinate relative to pointing direction
tform# = 1E
tunit# = deg
ttype# = DETY	  / coordinate relative to pointing direction
tform# = 1E
tunit# = deg
tform# = 1E
tunit# = deg
ttype# = ALT      / altitude of event
tform# = 1E
tunit# = deg
ttype# = AZ	  / azimuth of event
tform# = 1E
tunit# = deg
ttype# = COREX	  / core position in tilted system rel to array center
tform# = 1E
tunit# = m
ttype# = COREY	  / core position in tilted system rel to array center
tform# = 1E
tunit# = m
ttype# = CORE_ERR  / error in reconstructed core position
tform# = 1E
tunit# = m
tnull# = 0
ttype# = XMAX	  / height of shower-max
tform# = 1E
tunit# = m
ttype# = XMAX_ERR  / error in height of shower-max
tform# = 1E
tunit# = m
ttype# = ENERGY	  / shower reconstructed energy
tform# = 1E
tunit# = TeV
ttype# = ENERGY_ERR / error in shower reconstructed energy
tform# = 1E
tunit# = TeV
# OPTIONAL:
#ttype# = SHWIDTH   / Shower intrinsic width (if known)
#tform# = 1E
#tunit# = m
#tnull# = -1
#ttype# = SHLENGTH   / Shower intrinsic length (if known)
#tform# = 1E
#tunit# = m
#tnull# = -1
