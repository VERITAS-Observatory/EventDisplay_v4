################################################################################
#
# DATANAME CTA - hillas event columns that are telescope-specific 
#
# DESCRIPTION column definitions for Hillas-style reconstruction. Note
#  that here the vector length is set arbitrarily to 4 (it should be
#  the number of telescopes in the run, but is updated later by
#  fits_modify_vector_len() during processing.
#
# This is an include file - the extension must be defined before
# including this.
#
# CONTENT bintable definition
#
# CHANGES
#
#  - added impact parameter (ver 1.2) since this is difficult to 
#    calculate externally for each telescope
################################################################################
ttype# = HIL_TEL_LENGTH      / per-telescope length
tform# = 4E                  / initially 1 (updated later with correct number of tels)
tunit# = radians
ttype# = HIL_TEL_WIDTH       / per-telescope length
tform# = 4E     
tunit# = radians             
ttype# = HIL_TEL_PHI         / per-telescope phi angle of ellipse
tform# = 4E     
tunit# = degrees             
ttype# = HIL_TEL_COGX        / per-telescope centroid X position
tform# = 4E  
tunit# = degrees                
ttype# = HIL_TEL_COGY        / per-telescope centroid Y position
tform# = 4E     
tunit# = degrees             
ttype# = HIL_TEL_SKEW        / per-telescope skewness
tform# = 4E                  
ttype# = HIL_TEL_KURT        / per-telescope kurtosis
tform# = 4E                  
ttype# = HIL_TEL_SIZE        / per-telescope SIZE
tform# = 4E                  
ttype# = HIL_TEL_IMPACT      / per-telescope impact parameter
tform# = 4E                  
