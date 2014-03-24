################################################################################
#
# DATANAME CTA - telescope table
#
# DESCRIPTION table definition describing the array layout (properties
#  of each telescope in the array)
#
# CONTENT bintable definition
#
#
# TODO 
#
# - we could add corrected telescope pointing information here (e.g
#       where each telescope is really pointing, not just the nominal
#       value stored in the run header)
#
# CHANGES
#
################################################################################
XTENSION = BINTABLE     / Binary table extension
EXTNAME  = TELARRAY     / Extension name
TELESCOP = CTA
ARRAY    = unknown      / name of array layout for reference
OBS_ID   = 0            / name of observation this array corresponds to, or 0 if general
GEOLAT   = 0            / latitude of observatory
GEOLON   = 0            / longitude of observatory
ALTITUDE = 0            / altitude of observatory (km)
ttype# = TELID		/ telescope id number
tform# = 1I 
ttype# = CLASS		/ telescope type integer 
tform# = 1K
ttype# = SUBCLASS	/ telescope type integer 
tform# = 1K
ttype# = POSX		/ X position on ground rel to center of array
tform# = 1D
tunit# = m
ttype# = POSY		/ Y position on ground rel to center of array
tform# = 1D
tunit# = m
ttype# = POSZ		/ Z position on ground rel to center of array
tform# = 1D
tunit# = m
ttype# = MIRAREA	/ mirror area of the telescope
tform# = 1D
tunit# = m^2
ttype# = CAMAREA	/ camera plane area
tform# = 1D
tunit# = m^2
ttype# = FOCLEN		/ Focal Length 
tform# = 1D
tunit# = m
ttype# = FOV		/ Field of view
tform# = 1D
tunit# = deg
#======================================================
# extra camera information
#======================================================
ttype# = N_PIX     / number of pixels in camera
tform# = 1D
tunit# = deg
ttype# = PIX_SIZE  / Pixel size
tform# = 1D
tunit# = deg
ttype# = PIX_SEP   / Pixel spacing
tform# = 1D
tunit# = deg
