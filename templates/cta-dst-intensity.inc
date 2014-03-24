################################################################################
#
# DATANAME CTA - telescope-specific instensity statistics columns
#
# DESCRIPTION  
#  per-telescope parameters pertaining to intensity statistics
#
# NOTES:
#
#  - here the vector length is set arbitrarily to 4 (it should be
#  the number of telescopes in the run, but is updated later by
#  fits_modify_vector_len() during processing.
#
# - the INT_TEL_MAXPIX* and INT_TEL_MAXVAL* values could be done as 2D
#     arrays using the TDIMn keywords (e.g. (6,4) for 6 values and 4
#     telescopes, but that makes the reading software more
#     complicated...)
#
#
#
# This is an include file - the extension must be defined before
# including this.
#
# CONTENT bintable definition
#
# CHANGES
#
################################################################################
ttype# = INT_TEL_SIZE    / per-telescope total intensity (SIZE)
tform# = 4E              / (updated at write-time with correct number of tels)
ttype# = INT_TEL_SIZE_RMS    / per-telescope RMS of total intensity
tform# = 4E
ttype# = INT_TEL_NPIX   / number of pixels in image
tform# = 4I
ttype# = INT_TEL_MAXPIX1   / index of Nth maximum pixel in image
tform# = 4I
ttype# = INT_TEL_MAXPIX2   / index of Nth maximum pixel in image
tform# = 4I
ttype# = INT_TEL_MAXPIX3   / index of Nth maximum pixel in image
tform# = 4I
ttype# = INT_TEL_MAXPIX4   / index of Nth maximum pixel in image
tform# = 4I
ttype# = INT_TEL_MAXPIX5   / index of Nth maximum pixel in image
tform# = 4I
ttype# = INT_TEL_MAXVAL1   / value of Nth maximum pixel in image
tform# = 4E
ttype# = INT_TEL_MAXVAL2   / value of Nth maximum pixel in image
tform# = 4E
ttype# = INT_TEL_MAXVAL3   / value of Nth maximum pixel in image
tform# = 4E
ttype# = INT_TEL_MAXVAL4   / value of Nth maximum pixel in image
tform# = 4E
ttype# = INT_TEL_MAXVAL5   / value of Nth maximum pixel in image
tform# = 4E
