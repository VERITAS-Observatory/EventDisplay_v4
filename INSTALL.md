#  INSTALLATION

## Prerequisites

- ROOT must be installed 
  version >= 6.20

- SOFA library (http://www.iausofa.org/current_C.html) must be installed. Use the script in the $EVNDISPSYS directory:
```
./install_sofa.sh
```
Set the following environmental variable:  SOFASYS=$EVNDISPSYS/sofa

### VERITAS analysis

Requires installation of the VBF library (VERITAS bank format).
Note that for newer UNIX system, adaptations to C++17 standards are required.
Use e.g.
- https://github.com/VERITAS-Observatory/VBF/releases/tag/0.3.4-1-c%2B%2B17 for newer systems C++17 support
- https://github.com/VERITAS-Observatory/VBF/releases/tag/0.3.4-1 for all other systems

(see VERITAS internal wiki for all details)

### Optional

(3.0) for all FITS related output, cfitsio is needed (see (http://heasarc.gsfc.nasa.gov/fitsio/))

(3.1) GSL libraries (ROOT included gsl is fine)
      (http://www.gnu.org/software/gsl/)

## Environmental Variables

### Compiling and linking

ROOTSYS :   (required) ROOT installation; add $ROOTSYS/lib to $LD_LIBRARY_PATH and $ROOTSYS/bin to $PATH 
            (root should be compiled with minuit2, mysql, XML)

SOFASYS:    (required) Astronomy library from Sofa (use ./install_sofa.sh) for installation)

VBSYS :     (optional) VBF libraries (for VERITAS analysis only); add $VBFSYS/bin to $PATH and $VBFSYS/lib to $LD_LIBRARY_PATH

FITSSYS :   (optional) FITS libraries (optional, not needed in most cases)

GSLSYS :    (optional) GSL libraries

### Analysis

EVNDISPSYS : EVNDISP directory (scripts expect binaries in $EVNDISPSYS/bin and libraries in $EVNDISPSYS/lib) 

For root versions >=6.xx: add $EVNDISPSYS/obj to LD_LIBRARY_PATH

### Data directories

(different auxiliary data files are needed for the analysis (e.g. calibration files, detector geometry, etc))

The following setup are for an environment where several users for example are analysis the same raw data
(would be in $VERITAS_DATA_DIR), but having their analysis results written to their own
directories ($VERITAS_USER_DATA_DIR). 
Note that e.g. $VERITAS_DATA_DIR and $VERITAS_USER_DATA_DIR can point to the same directory.

For VERITAS analysis, the following directories are needed:

VERITAS_EVNDISP_AUX_DIR:  directory with all auxiliary data like calibration files, lookup tables, effective areas, etc
VERITAS_DATA_DIR :        directory containing the raw telescope data or input simulation files 
VERITAS_USER_DATA_DIR :   user data directory: containing output files from this analysis package (most of them in root format)
VERITAS_IRFPRODUCTION_DIR : directory used in IRF production (not needed for normal data analysis; ignore it if you do not know it)

To set the variables for VERITAS:

```
./setObservatory.sh VERITAS
```

# Compiling

Check your systems configuration:
```
make config
```
Compiling and installing:

in $EVNDISPSYS:

for VERITAS analysis type:
```
make VTS
```

## Makefile targets

- all	make all executable and libraries
- clean
- install
- VTS	make all VERITAS relevant binaries/libraries
