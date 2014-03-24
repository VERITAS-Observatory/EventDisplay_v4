################################################################################
#
# DATANAME CTA - monte-carlo information
#
# DESCRIPTION 
#
#
# information pertaining to monte-carlo simulation parameters used to
# generate an eventlist. This is not intended to contain all possible
# simulation parameters, rather just ones needed for forward-folding
# spectrum generation. This is complimentary to the TELARRAY table.
#
# CONTENT bintable definition
#
# CHANGES
#
################################################################################
XTENSION = BINTABLE     / Binary table extension
EXTNAME  = MCINFO       / Extension name
SHWRSIM  = UNKNOWN      / shower simulation program (e.g. 'corsika')
SHWRVER  = 0            / version number of shower sim
DETSIM   = UNKNOWN      / detector simulation program (e.g. 'sim_telarray')
DETVER   = 0            / version number of detector sim
NTELS    = 0            / number of tels in simulations
ATMOMODL = 0            / atmosphere model
B_FIELD  = 0            / magnetic field strength    [uT]
B_INC    = 0            / magnetic field inclination [deg]
B_DEC    = 0            / magnetic field declanation [deg]
ALTITUDE = 0            / altitude of observatory [deg]
INJECTHT = 0            / injection height [m]
INTDEPTH = 0            / 1st interaction depth [g/cm^2]
COMMENT This table describes the input parameters of the monte-carlo
COMMENT simulations that were used to produce an event-list.
COMMENT Multiple simulation types and energy bins may be combined, 
COMMENT each with one entry in the MCINFO table entry
ttype# = MC_RUN_ID   / Monte-carlo run ID
tform# = 1I   
ttype# = MODE        / mode bitmask (bit1 = diffuse)
tform# = 8x
ttype# = N_EVENTS    / number of thrown events in this group
tform# = 1V
ttype# = N_REUSED    / number of times showers were reused
tform# = 1V
ttype# = PRIM_ID     / type of primary (see Corsika types)
tform# = 1I
ttype# = E_MIN       / Minimum simulated energy 
tform# = 1E   
tunit# = TeV       
ttype# = E_MAX       / Maximum simulated energy 
tform# = 1E          
tunit# = TeV
ttype# = INDEX       / spectral index of thrown events (E^{-INDEX})
tform# = 1E   
tunit# = TeV       
ttype# = CONE_MIN    / view-cone minimum value           
tform# = 1E   
tunit# = deg
ttype# = CONE_MAX    / view-cone opening angle
tform# = 1E   
tunit# = deg
ttype# = CORE_MIN    / thrown core-radius min
tform# = 1E   
tunit# = deg
ttype# = CORE_MAX    / thrown core-radius max       
tform# = 1E   
tunit# = deg
