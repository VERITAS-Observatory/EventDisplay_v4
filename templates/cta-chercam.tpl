################################################################################
#
# DATANAME CTA - cherenkov camera definition
#
# DESCRIPTION table describing the pixel layout of a chenkov camera
#
# CONTENT bintable definition
#
#
# TODO 
#
#
# CHANGES
#
################################################################################
XTENSION = BINTABLE     / Binary table extension
EXTNAME  = CHERCAM     / Extension name
TELESCOP = CTA
ARRAY    = unknown      / name of array layout for reference
ttype# = CAM_ID		/ camera id number 
tform# = 1I 
ttype# = PIX_ID		/ pixel id number
tform# = 1K
ttype# = PIX_POSX	/ X position of pixel center in camera 
tform# = 1D
tunit# = m
ttype# = PIX_POSY	/ Y position of pixel center in camera
tform# = 1D
tunit# = m
ttype# = PIX_AREA	/ pixel sensitive area 
tform# = 1D
tunit# = m^2
ttype# = PIX_DIAM	/ physical diameter of pixel
tform# = 1D
tunit# = m
ttype# = PIX_NEIG	/ neighbor pixel IDs of this pixel, -1 if blank
tform# = 6I
ttype# = DUMMY
tform# = 1I