################################################################################
#
# DATANAME CTA - monte-carlo event columns
#
# DESCRIPTION column definitions for monte-carlo event parameters
#
# This is an include file - the extension must be defined before
# including this.
#
# CONTENT bintable definition
#
# CHANGES
#
################################################################################
ttype# = MC_EVENTID    / event ID number from simulation
tform# = 1J
ttype# = MC_SHOWERID    / shower ID number from simulation
tform# = 1J
ttype# = MC_PRIMID      / type of primary particle
tform# = 1I / short
ttype# = MC_ENERGY      / true energy of particle
tform# = 1E
tunit# = TeV
ttype# = MC_ALT         / true altitude
tform# = 1E
tunit# = deg
ttype# = MC_AZ          / true azimuth 
tform# = 1E
tunit# = deg
ttype# = MC_COREX       / true core X position of shower axis
tform# = 1E
tunit# = m
ttype# = MC_COREY       / true core Y position of shower axis
tform# = 1E
tunit# = m
ttype# = MC_COREZ       / true core Y position of shower axis
tform# = 1E
tunit# = m
ttype# = MC_FIRSTINT    / altitude (asl) of first interaction
tform# = 1E
tunit# = m
ttype# = MC_XSTART      / atmospheric depth of particle
tform# = 1E
tunit# = g/cm^2
ttype# = MC_XMAX        / atmospheric depth of shower max
tform# = 1E
tunit# = g/cm^2
