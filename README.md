# Eventdisplay - An Analysis and Reconstruction Package for VERITAS

**Development version main branch - not to be used for science analysis**

Please check the [release pages](https://github.com/VERITAS-Observatory/EventDisplay_v4/releases) for the most recent versions.

For older versions of Eventdisplay, see the release pages on the VERITAS wiki (private pages):
https://veritas.sao.arizona.edu/wiki/index.php/Eventdisplay_Manual#Versions_of_eventdisplay


## Overview

Eventdisplay is a reconstruction and analysis pipline for data of
Imaging Atmospheric Cherenkov Telescopes (IACT).
It has been primarily developed for VERITAS and CTA analysis.

Original developers: Gernot Maier and Jamie Holder

In case Eventdisplay is used in a research project, please cite the 
following publication:

Maier, G.; Holder, J., Eventdisplay: An Analysis and Reconstruction Package for 
Ground-based Gamma-ray Astronomy,  35th International Cosmic Ray Conference.
10-20 July, 2017. Bexco, Busan, Korea, Proceedings of Science, Vol. 301.
Online at [https://pos.sissa.it/cgi-bin/reader/conf.cgi?confid=301], id.747
[https://arxiv.org/abs/1708.04048]

For guidelines on installation, see INSTALL. For further information, 
see files in README directory

## Documentation

- INSTALL: information on installation the analysis package, dependencies, environmental variables
- README.VERITAS.quick_summary: description of a typical VERITAS analysis
- AUTHORS: author description

Description and command line options for the different software parts:

- README.EVNDISP
- README.EVNDISP.commandline
- README.MSCW_ENERGY
- README.ANASUM
- README.EFFECTIVEAREA
- README.ANALYSISLIBRARY
- README.SCRIPTS
- README.MACROS

## Licence

License: BSD-3 (see LICENCE file)

## The Eventdisplay Ecosystem

Reconstruction and analysis can be complex; it requires inputs from different sources and execution of several indedependent stages.
Eventdisplay is in use since roughly 2004 and an ecosystem of libaries and repositories grew around the core code base. 
Below an overview of those repositories. 
Some are internal to VERITAS and not accessible to the general public.

For almost every use case, Eventdisplay consists of at least three major components: 
- the code (Eventdisplay), 
- a library of scripts,
- a set of auxiliary files.

Care should be taken in using the correct versions (releases, tags, branches) combining these three types of repositories.
A blending of different versions of components will lead to incorrect results.

### Code, tools, library

The core library consist of all code, tools, and libraries required to run the analysis.

Private (VERITAS) repository (to be used for VERITAS analysis): <https://github.com/VERITAS-Observatory/EventDisplay_v4>

### Analysis scripts

Typical use cases for Eventdisplay require the processing of many files.
A library of scripts for the efficient execution is available and recommended to be used as the main access to the tools described in the section above.

Repository for scripts: <https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisScripts_VTS> 

### Auxiliary files for parameters, definitions, calibration values

The reconstruction of analysis requires information on the instrument (e.g., telescope positions), access information to data bases, parameters for the analysis (e.g., image cleaning parameters or instruction for the gamma/hadron separation), or basic calibration values.

Repository for auxiliary files: (private repository): <https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisFiles_VTS>

### Release tests

Reconstruction and analysis are complex and a series of tests are required before the release of a new version of Eventdisplay.

Release repository: (private repository): <https://github.com/VERITAS-Observatory/EventDisplay_ReleaseTests>


For any questions, contact Gernot Maier (gernot.maier@desy.de)
