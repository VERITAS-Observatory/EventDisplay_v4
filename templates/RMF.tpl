#
# Template for a real-data DST 
# (events + telescope parameters)
#
#
XTENSION= 'BINTABLE'           / binary table extension
EXTNAME = 'MATRIX  '           / extension name
BITPIX  =                    8 / array data type
NAXIS   =                    0 / number of array dimensions
PCOUNT  =                    0 / number of group parameters
GCOUNT  =                    1 / number of groups
TTYPE#  = 'ENERG_LO'
TFORM#  = 'E       '
TUNIT#  = 'TeV     '
TTYPE#  = 'ENERG_HI'
TFORM#  = 'E       '
TUNIT#  = 'TeV     '
TTYPE#  = 'N_GRP   '
TFORM#  = 'I       '
TUNIT#  = 'TeV     '
TTYPE#  = 'F_CHAN  '
TFORM#  = 'I       '
TUNIT#  = 'TeV     '
TTYPE#  = 'N_CHAN  '
TFORM#  = 'I       '
TUNIT#  = 'TeV     '
TTYPE#  = 'MATRIX  '
TFORM#  = '100E    '
HDUCLASS= 'OGIP    '           / Organization of definition
HDUCLAS1= 'RESPONSE'           / dataset relates to spectral response
HDUCLAS2= 'RSP_MATRIX'         / dataset is a response matrix
HDUCLAS3= 'REDIST  '           / photon redistribution matrix
HDUVERS = '1.3.0   '           / Version of format (OGIP memo CAL/GEN/92-002a)
HDUVERS1= '1.3.0   '           / Obsolete - included for backwards compatibility
CHANTYPE= 'PI      '           / Required keyword, X-ray relic
TELESCOP= 'HESS    '           / Mission name
INSTRUME= 'unknown '           / Instrument name