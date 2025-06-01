# Eventdisplay: An Analysis and Reconstruction Package for VERITAS

[![DOI](https://zenodo.org/badge/221041866.svg)](https://zenodo.org/badge/latestdoi/221041866)
[![ASCL](https://img.shields.io/badge/ascl-2212.002-blue.svg?colorB=262255)](https://ascl.net/2212.002)
[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![CI](https://github.com/VERITAS-Observatory/EventDisplay_v4/actions/workflows/ci.yml/badge.svg)](https://github.com/VERITAS-Observatory/EventDisplay_v4/actions/workflows/ci.yml)

## Overview

Eventdisplay is a reconstruction and analysis pipeline for data of
Imaging Atmospheric Cherenkov Telescopes.
It has been primarily developed for VERITAS and CTA analysis and publications.
This repository contains the Eventdisplay version used for VERITAS analysis (see [here](https://github.com/Eventdisplay/Eventdisplay) for the CTA version).

* Original developers: Gernot Maier and Jamie Holder
* Authors and contributors: [CITATION.cff](CITATION.cff)
* License: [LICENSE](LICENSE)
* Contributing: [CONTRIBUTING.md](CONTRIBUTING.md)

In case Eventdisplay is used in a research project, please cite this repository using the [Zenodo entry](https://zenodo.org/badge/latestdoi/221041866) and the following publication:

```text
Maier, G.; Holder, J., Eventdisplay: An Analysis and Reconstruction Package for
Ground-based Gamma-ray Astronomy,  35th International Cosmic Ray Conference.
10-20 July, 2017. Bexco, Busan, Korea, Proceedings of Science, Vol. 301.
Online at [https://pos.sissa.it/cgi-bin/reader/conf.cgi?confid=301], id.747
[https://arxiv.org/abs/1708.04048]
```

The package consists of several analysis steps and tools:

1. `evndisp`: calibrate and parametrize images, event reconstruction, stereo analysis
2. `trainTMVAforAngularReconstruction`: train boosted decision trees for direction and energy reconstruction
3. `mscw_energy`: fill and use lookup tables for mean scaled with and length calculation, energy reconstruction, stereo reconstruction
4. `trainTMVAforGammaHadronSeparation`: train boosted decision trees for gamma/hadron separation
5. `makeEffectiveArea`: calculation of the instrument response functions (effective areas, angular point-spread function, energy resolution)
6. `makeRadialAcceptance`: calculation of radial camera acceptance from data files
7. `anasum`: analysis to calculate sky maps and spectral energy distribution
8. `libVAnaSum`: shared library tools (e.g., plot instrument response function, spectral energy distributions, light curves, sky maps

## Releases

Check the [release pages](https://github.com/VERITAS-Observatory/EventDisplay_v4/releases) for the most recent versions to be used for VERITAS publications.
For older versions of Eventdisplay, see the release pages on the [VERITAS wiki (internal pages)](https://veritas.sao.arizona.edu/wiki/index.php/Eventdisplay_Manual#Versions_of_eventdisplay).

## The Eventdisplay Ecosystem

Reconstruction and analysis are complex and require inputs from different sources and execution of several interdependent stages.

Eventdisplay consists of the following main components:

* [Eventdisplay](https://github.com/VERITAS-Observatory/EventDisplay_v4): the core library with code, tools, and analysis libraries required to run the analysis
* [Eventdisplay_AnalysisScripts_VTS](https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisScripts_VTS): a library of scripts to use Eventdisplay efficiently on a computing cluster
* [Eventdisplay_AnalysisFiles_VTS](https://github.com/VERITAS-Observatory/Eventdisplay_AnalysisFiles_VTS): a set of auxiliary files with information on the instrument (e.g., telescope positions), access information to data bases, parameters for the analysis (e.g., image cleaning parameters or instruction for the gamma/hadron separation), or basic calibration values (note that this is a VERITAS private repository)
* [V2DL3](https://github.com/VERITAS-Observatory/V2DL3): a converter of Eventdisplay data products to GADF DL3 Format to be used in science tools like [gammapy](https://github.com/gammapy/gammapy).
* [Eventdisplay_Preprocessing](https://github.com/VERITAS-Observatory/EventDisplay_Preprocessing/blob/main/README.md): Documentation and scripts related to the pre-processing all VERITAS observations with Eventdisplay.

Additional components:

* release test macros and results (see internal GitHub repository [Eventdisplay_ReleaseTests](https://github.com/VERITAS-Observatory/EventDisplay_ReleaseTests_code).

## Documentation

* [INSTALL.md](INSTALL.md): installation, dependencies, environmental variables
* [README.VERITAS.quick_summary](README/README.VERITAS.quick_summary): description of a typical VERITAS analysis

Extensive documentation on how to use Eventdisplay are available through the [VERITAS internal wiki pages](https://veritas.sao.arizona.edu/wiki/Eventdisplay_Manual).

## Support

* VERITAS internal [ELOG](http://veritas.sao.arizona.edu/private/elog/Eventdisplay-WG/) used for announcements, discussions, questions
* Bugs and issues should be reported through the [GitHub issue tracker](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues)

For any questions, contact Gernot Maier.

## Development

astyle is used for source code formatting since 03/2014
http://astyle.sourceforge.net/

Usage:

do either

```console
make formatSourceCode
```

 or

```console
astyle  --options=./.astylerc <.cpp, .h or .C file>
```

All options are saved in the .astylerc file.
