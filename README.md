# Eventdisplay - An Analysis and Reconstruction Package for VERITAS

[![DOI](https://zenodo.org/badge/221041866.svg)](https://zenodo.org/badge/latestdoi/221041866)
[![CI](https://github.com/VERITAS-Observatory/EventDisplay_v4/actions/workflows/ci.yml/badge.svg)](https://github.com/VERITAS-Observatory/EventDisplay_v4/actions/workflows/ci.yml)

* Authors and contributors: [CITATION.cff](CITATION.cff)
* License: [LICENSE](LICENSE)

Please check the [release pages](https://github.com/VERITAS-Observatory/EventDisplay_v4/releases) for the most recent versions to be used for VERITAS publications.

For older versions of Eventdisplay, see the release pages on the VERITAS wiki (private pages):
https://veritas.sao.arizona.edu/wiki/index.php/Eventdisplay_Manual#Versions_of_eventdisplay


## Overview

Eventdisplay is a reconstruction and analysis pipeline for data of
Imaging Atmospheric Cherenkov Telescopes (IACT).
It has been primarily developed for VERITAS and CTA analysis and used in
many VERITAS and CTA publications. 
This repository contains the Eventdisplay version used for VERITAS analysis.
For the CTA version, please go to [https://github.com/Eventdisplay/Eventdisplay](https://github.com/Eventdisplay/Eventdisplay).

Original developers: Gernot Maier and Jamie Holder

In case Eventdisplay is used in a research project, please cite this repository and
the following publication:

Maier, G.; Holder, J., Eventdisplay: An Analysis and Reconstruction Package for 
Ground-based Gamma-ray Astronomy,  35th International Cosmic Ray Conference.
10-20 July, 2017. Bexco, Busan, Korea, Proceedings of Science, Vol. 301.
Online at [https://pos.sissa.it/cgi-bin/reader/conf.cgi?confid=301], id.747
[https://arxiv.org/abs/1708.04048]

For guidelines on installation, see [INSTALL.md](INSTALL.md). For further information, 
see files in [README](./README) directory

## Documentation

- [INSTALL.md](INSTALL.md): information on installation the analysis package, dependencies, environmental variables
- [README.VERITAS.quick_summary](README/README.VERITAS.quick_summary): description of a typical VERITAS analysis

Description and command line options for the different software parts:

- [README.EVNDISP](README/README.EVNDISP)
- [README.EVNDISP.commandline](README/README.EVNDISP.commandline)
- [README.MSCW_ENERGY](README/README.MSCW_ENERGY)
- [README.ANASUM](README/README.ANASUM)
- [README.EFFECTIVEAREA](README/README.EFFECTIVEAREA)
- [README.ANALYSISLIBRARY](README/README.ANALYSISLIBRARY)


## The Eventdisplay Ecosystem

Reconstruction and analysis can be complex; it requires inputs from different sources and execution of several interdependent stages.
Eventdisplay is in use since roughly 2004 and an ecosystem of libraries and repositories grew around the core code base. 
Below an overview of those repositories. 
Some are internal to VERITAS and not accessible to the general public.

For almost every use case, Eventdisplay consists of at least three major components: 
- the code (Eventdisplay), 
- a library of scripts (see [Eventdisplay_AnalysisScripts_VTS](https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisScripts_VTS))
- a set of auxiliary files (see internal GitHub repository [Eventdisplay_AnalysisFiles_VTS](https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisFiles_VTS))
- release test macros and results (see internal GitHub repository [Eventdisplay_ReleaseTests](https://github.com/VERITAS-Observatory/Eventdisplay_ReleaseTests))

Care should be taken in using the correct versions (releases, tags, branches) combining these three types of repositories.
A blending of different versions of components will lead to incorrect results.

### Code, tools, library

The core library consist of all code, tools, and libraries required to run the analysis.

### Analysis scripts

Typical use cases for Eventdisplay require the processing of many files.
A library of scripts for the efficient execution is available and recommended to be used as the main access to the tools described in the section above.

Repository for scripts: [https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisScripts_VTS](https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisScripts_VTS)

### Auxiliary files for parameters, definitions, calibration values

The reconstruction of analysis requires information on the instrument (e.g., telescope positions), access information to data bases, parameters for the analysis (e.g., image cleaning parameters or instruction for the gamma/hadron separation), or basic calibration values.

Repository for auxiliary files: (private repository): [https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisFiles_VTS](https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisFiles_VTS)

### Release tests

Reconstruction and analysis are complex and a series of tests are required before the release of a new version of Eventdisplay.

Release repository: (private repository): [https://github.com/VERITAS-Observatory/EventDisplay_ReleaseTests](https://github.com/VERITAS-Observatory/EventDisplay_ReleaseTests)

### Converter to GADF DL3 Format

see [https://github.com/VERITAS-Observatory/V2DL3](https://github.com/VERITAS-Observatory/V2DL3)

For any questions, contact Gernot Maier (gernot.maier@desy.de)
