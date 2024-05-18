#  INSTALLATION

Eventdisplay is a C++ based library and designed to run in typical Linux environments. It has not been tested on MacOS (use Docker containers in this instance).

## Software required

### ROOT

CERN's [ROOT](https://root.cern.ch/) library for I/O, histogramming, and statistical applications:

- ROOT versions >= 6.28
- the first-stage tool `evndisp` requires ROOT compiled with mysql for access to the VERITAS database. Pre-compiled version of ROOT (downloaded from [here](https://root.cern/install/)) have mysql installed. If building from source, ensure the mysql dependencies are installed and compiler flags are added (see [root installation page](https://root.cern/install/build_from_source/)). All other stages of Eventdisplay do not required mysql - meaning e.g., the conda-based installation of Eventdisplay is fine.
- paths for ROOT should be set through e.g.,

```
export ROOTSYS=<Path to ROOT installation>/root/
cd $ROOTSYS
source ./bin/thisroot.sh
```

- test your ROOT installation using the `root-config` tool, which should be accessible from any directory (through the `PATH` variable) if the system is setup correctly:

```
root-config --version
root-config --has-mysql
```

### SOFA

[SOFA](http://www.iausofa.org/current_C.html) for all astronometry transformations.

Download and install using this script in the $EVNDISPSYS directory:

```
./install_sofa.sh
```

Set the following environmental variable: `SOFASYS=$EVNDISPSYS/sofa`

### VBF (evndisp stage only)

The first-stage tool `evndisp` requires the [VBF](https://github.com/VERITAS-Observatory/VBF) (VERITAS bank format) library to read VERITAS raw data files.

- use VBF version VBF >= 0.3.4
- https://github.com/VERITAS-Observatory/VBF/releases/tag/0.3.4-1-c%2B%2B17 for newer Linux systems C++17 support
- https://github.com/VERITAS-Observatory/VBF/releases/tag/0.3.4-1 for all other systems

Simple installation instructions are found in `VBF/README`. To install in a local directory instead of the first installation command in VBF/README use `$ /.configure --prefix=<local directory>` and change the environment variable `VBFSYS` to `<local directory>`.

Set the following environmental variable: `VBFSYS=<directory with VBF installation>`

### GSL (optional)

GSL libraries are used for very specific tasks (Hough muon calibration, likelihood fitter) and are not required for most users. GSL is included in all pre-compiled ROOT versions; for building from source, see the [gsl web page](http://www.gnu.org/software/gsl/).

### cfitsio (optional)

FITS related output as used for the next-day analysis and requires installation of cfitsio (see http://heasarc.gsfc.nasa.gov/fitsio/).

## Environmental Variables

Eventdisplay can be used efficiently with the correct environmental variables set.

### Compiling and Linking

ROOTSYS :   (required) ROOT installation; add $ROOTSYS/lib to $LD_LIBRARY_PATH and $ROOTSYS/bin to $PATH

SOFASYS:    (required) Astronomy library from Sofa

VBSYS :     (optional) VBF libraries (for `evndisp` analysis only); add $VBFSYS/bin to $PATH and $VBFSYS/lib to $LD_LIBRARY_PATH

FITSSYS :   (optional, not needed in most cases) FITS libraries

GSLSYS :    (optional, not needed in most cases) GSL libraries

### Analysis

EVNDISPSYS : Eventdisplay code directory (scripts expect binaries in $EVNDISPSYS/bin and libraries in $EVNDISPSYS/lib). Add $EVNDISPSYS/obj to LD_LIBRARY_PATH.

EVNDISPSCRIPTS: Eventdisplay scripts directory

### Data directories

Assume a computing environment, where several users are analysis the same raw data
(would be in $VERITAS_DATA_DIR), but having their analysis results written to their own
directories ($VERITAS_USER_DATA_DIR).
Note that $VERITAS_DATA_DIR and $VERITAS_USER_DATA_DIR can point to the same directory.

- VERITAS_EVNDISP_AUX_DIR:  directory with all auxiliary data like calibration files, lookup tables, effective areas, etc
- VERITAS_DATA_DIR :        directory containing the raw telescope data or input simulation files
- VERITAS_USER_DATA_DIR :   user data directory: containing output files from this analysis package
- VERITAS_USER_LOG_DIR :    user log file directory: log files and temporary scripts are written to this directory

To set the variables for VERITAS:

```
./setObservatory.sh VERITAS
```

# Compiling

Check your systems configuration:

```
make config
```

Compare the output with the requirements on software and environmental variable settings described above.

In the main Eventdisplay directory ($EVNDISPSYS is pointing to this directory), compile all Eventdisplay binaries with

```
source ./setObservatory.sh VTS
make VTS
```

If you are working on a computing with several cores, this can be accelerated by e.g. compiling with four cores in parallel:

```
source ./setObservatory.sh VTS
make -j 12 VTS
```

To compile a single component of the software only, e.g.:

```
source ./setObservatory.sh VTS
make slib
```

Use `make clean` to remove any files produced in earlier compilation runs.

# Troubleshooting

- many compilation issues are related to incorrect settings of the environmental variables, especially `ROOTSYS` and `VBFSYS`.
- ask via ELOG, slack, or GitHub issues for persistent issues
- check your installation, it should look something like this (with different directories):

```
CONFIGURATION SUMMARY FOR EVNDISP version 490
======================================================

g++ 4.8.5 on x86_64-redhat-linux Linux
    4 8
    -O3 -g -Wall -fPIC -fno-strict-aliasing -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_SOURCE -D_LARGEFILE64_SOURCE -I. -I./inc/  -DRUNWITHDB  -DASTROSOFA  -pthread -m64 -std=c++11 -I/afs/ifh.de/group/cta/cta/software/root/root-6.24.06_build/include -I/afs/ifh.de/group/cta/cta/software/root/root-6.24.06_build/include/TMVA -I/afs/ifh.de/group/cta/VERITAS/software/VBF-0.3.4-SL6/include/VBF/ -I/usr/include -I/afs/ifh.de/group/cta/scratch/maierg/EVNDISP/EVNDISP-400/GITHUB_Eventdisplay/EventDisplay_v490-dev//sofa/include/
    -L/afs/ifh.de/group/cta/cta/software/root/root-6.24.06_build/lib -lGui -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -pthread -lm -ldl -rdynamic -lMLP -lTMVA -lMinuit -lXMLIO -lSpectrum -lgsl -lgslcblas -lm -L/afs/ifh.de/group/cta/scratch/maierg/EVNDISP/EVNDISP-400/GITHUB_Eventdisplay/EventDisplay_v490-dev//sofa/lib -lsofa_c

using root version 6.24/06
    yes
    /afs/ifh.de/group/cta/cta/software/root/root-6.24.06_build

evndisp with GSL libraries (used in Hough muon calibration, likelihood fitter)
   GSL
   -I/usr/include -lgsl -lgslcblas -lm

evndisp with VBF support
    VBFSYS /afs/ifh.de/group/cta/VERITAS/software/VBF-0.3.4-SL6
    VBFCFLAGS -I/afs/ifh.de/group/cta/VERITAS/software/VBF-0.3.4-SL6/include/VBF/
    VBFLIBS -L/afs/ifh.de/group/cta/VERITAS/software/VBF-0.3.4-SL6/lib   -lVBF -lbz2 -lz

evndisp with database (mysql) support

no FITS support enabled

Astronometry with SOFALIB /afs/ifh.de/group/cta/scratch/maierg/EVNDISP/EVNDISP-400/GITHUB_Eventdisplay/EventDisplay_v490-dev//sofa
TSpectrum available for low-gain calibration

Testing EVNDISP environmental variables (for VTS):
----------------------------------------------------------
EVNDISPSYS set to /afs/ifh.de/group/cta/scratch/maierg/EVNDISP/EVNDISP-400/GITHUB_Eventdisplay/EventDisplay_v490-dev/
VERITAS_EVNDISP_AUX_DIR set to /lustre/fs23/group/veritas/Eventdisplay_AnalysisFiles/v490/
VERITAS_DATA_DIR set to /lustre/fs24/group/veritas/
VERITAS_USER_DATA_DIR set to /lustre/fs23/group/veritas/users/maierg
VERITAS_IRFPRODUCTION_DIR set to /lustre/fs23/group/veritas/IRFPRODUCTION
```
