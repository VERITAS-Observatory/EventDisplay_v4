################################################################################
#
# DATANAME CTA - telescope and pixel wise intensity data
#
# DESCRIPTION 
#
# This is an include file - the extension must be defined before
# including this.
#
# NOTES
# In even lower-level data, the shower-images themselves can optionally be stored
# in this same table. To reduce the size of the data, only cleaned pixels may be
# stored by first listing the non-zero pixel numbers as an array TEL_IMG_IPIX and
# the corresponding intensities in TEL_IMG_INT.
#
# TEL_IMG_IPIX & TEL_IMG_INT are fixed length arrays of size of the total number
# of pixels in the camera. Empty entries shall be filled with 0 and the file shall
# be compressed.
#
# CONTENT bintable definition
#
# CHANGES
#
# 2011-09-12: changed to using variable-length arrays for TEL_IMG_IPIX
#  and TEL_IMG_INT
#
################################################################################
ttype# = TEL_IMG_NPIX  / number of pixels in image
tform# = 1I
ttype# = TEL_IMG_IPIX  / list of pixel indices in image
tform# = 1PI(2048)
ttype# = TEL_IMG_INT / list of pixel intensities in image (P.E)
tform# = 1PE(2048)
tunit# = p.e.
