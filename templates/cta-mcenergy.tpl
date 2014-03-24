################################################################################
#
# DATANAME CTA - monte-carlo thrown energy histogram
#
# DESCRIPTION 
#
# Histogram of number of events thrown for each energy bin
# (in case it's not an exact power law, this is a better thing to use for effective
# area calculations)
# 
# 
#
# CONTENT bintable definition
#
# CHANGES
#
################################################################################
XTENSION = BINTABLE     / Binary table extension
EXTNAME  = MCENERGY       / Extension name
ttype# = E_MIN       / bin lower edge
tform# = 1E   
tunit# = TeV 
ttype# = E_MAX       / bin upper edge
tform# = 1E          
tunit# = TeV
ttype# = N           / number of events in bin
tform# = 1E          
tunit# = counts
ttype# = N_ERR       / error on bin
tform# = 1E          

