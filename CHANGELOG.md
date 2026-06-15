# Changelog

All notable changes to the Eventdisplay_v4 project will be documented in this file.
Changes for upcoming releases can be found in the [docs/changes](docs/changes) directory.
Note that changes before release v492.0 are not documented here, but can be found in the
[GitHub repository](https://github.com/VERITAS-Observatory/EventDisplay_v4/releases).

This changelog is generated using [Towncrier](https://towncrier.readthedocs.io/).

<!-- towncrier release notes start -->

## [v492.0-rc5](https://github.com/VERITAS-Observatory/EventDisplay_v4/releases/tag/v492.0-rc5) - 2026-06-15

### Bugfixes

- Fixes long list of bugs listed in [Issue #352](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/352).
  Fix A2: `VTraceHandler::calculateTraceSum_slidingWindow()` fRaw branch returned `FADC[1]` (second FADC sample, ped-subtracted) instead of the accumulated raw charge sum. Affected pedestal calculation path. ([#351](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/351))
- Bug fix: non-XGB jobs no longer probe XGB sidecar files (E4)

  XGB sidecar files (stereo and gamma-hadron) are now opened only when the active
  reconstruction method and gamma-hadron cut type actually require them, eliminating
  spurious ROOT file-open errors in non-XGB jobs. ([#352](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/352))
- Bug fix: explicit NegWeightTreatment for TMVA Gradient BDT angular reconstruction

  Added `NegWeightTreatment=Pray` to the default TMVA options in
  `trainTMVAforAngularReconstruction` to match actual TMVA behaviour:
  `BoostType=Grad` does not support `InverseBoostNegWeights` (TMVA's global
  default) and silently replaces it with `Pray`; the options string now
  declares this explicitly so the training configuration matches what is used. ([#353](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/353))

### Maintenance

- Updates to allow for compilation with ROOT >=v6.38. Adjustment of CI. ([#358](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/358))


## [v492.0-rc4](https://github.com/VERITAS-Observatory/EventDisplay_v4/releases/tag/v492.0-rc4) - 2026-05-17

### New Feature

- Improve DispBDT reconstruction by adding `loss^2` and `loss*dist` as training variables. This improves sensitivity to truncated images. ([#318](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/318))
- Feature: Machine learning interface for direction reconstruction

  - Adds an interface to run machine learning models (e.g., scikit-learn).
  - Provides an example using XGBoost regression trees to estimate directions in camera coordinates.
  - Introduces a Python environment and a scripts directory to support training and inference.

  The new XGBoost-based training and reconstruction methods estimate the gamma‑ray arrival direction in camera coordinates. They support `DispBDT` and intersection methods, combining image parameters to compute `Xoff`/`Yoff`.

  Compared to previous approaches that averaged over all `DispBDT` directions, this implementation better handles edge cases (e.g., truncated images).

  ([#322](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/322))
- Add print out average run zenith angle to `printRunParameter` option `-runinfo`. ([#334](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/334))
- Improve TMVA interpolation from average zenith to average airmass. ([#336](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/336))
- Allow to choose energy and direction reconstruction methods:

  ```
  Stereo reconstruction
  ---------------------
  Method ids: 0 (DispBDT), 1 (LT Tables), 2 (XGB stereo)
  * ENERGYRECONSTRUCTIONMETHOD 0
  Method ids:  0 (DispBDT), 1 (Intersection Method), 2 (XGB stereo)
  * DIRECTIONRECONSTRUCTIONMETHOD 0
  ```

  Consistently use `CData` methods to get variables.

  ([#339](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/339))
- Improve mscw plotting macro for consistent comparison of different analysis sets. ([#340](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/340))
- Introduction of average zenith angle calculation per run in VTMVAEvaluator (fAverageZenithPerRun, calculate_average_zenith_angle()). ([#281](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/281))
- Introduced new functionality for reconstructing events using any three telescopes out of four (primarily for Monte Carlo data and effective area calculations).
  Added new class: `VMeanScaledVariables` for mean scaled variable calculations.
  Updated VDispAnalyzer and VSimpleStereoReconstructor to support new reconstruction modes and calculation methods.
  Updated VInstrumentResponseFunctionRunParameter and related code to include new parameters for 3-telescope reconstruction. ([#289](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/289))
- Add new command line parameter `--minpedestalevents` to set the minimum number of required pedestal events (default: 50). ([#298](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/298))

### Maintenance

- Improve efficiency of IRF calculation (lower storage footprint; introduce possibility to re-calculate all stereo parameters (MSCW, MSCL, emission height, direction, core, energy) on the `mscw_energy` level for 3-telescope events from 4-telescope MC simulations. This is for MC events only and used to simplify and accelerate the analysis required for the generation of effective areas.) ([#293](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/293))
- - reduced storage impact of `mscw_energy` branch (mostly due to change of doubles to floats; removal of unused variables); reduction is on storage requirements is ~25%.
  - efficiency improvement in running `mscw_energy` -> 20% faster
  - fine-tuning of docker build scripts to improve build efficiency
  - add code to redo stereo reconstruction to `CData.cpp`. Functions are named `reconstruct_3tel_images*`
  - generalized calculation of MSCW, MSCL, MWR, MWL variables in new classes `VMeanScaledVariables`
  - allow to recalculate `dispBDTDirection` and `dispBDTEnergy` in `VDispAnalyzer` from values per image
  - several important improvements and bug fixes in stereo reconstruction code in `VTableLookupDataHandler`:
  	- `Chi2` from direction reconstruction not correctly filled from `showerpars` trees
  	- bug fix in setting of `NImages` and selected telescopes for 3-telescope runs (function `fill_selected_images_before_redo_stereo_reconstruction`)
  - clarified that `Erec` is from `dispBDTEnergy` while `ErecS` is from simple lookup table reconstruction. Change default values for `EnergyReconstructionMethod` to `Erec` (method ID `0`) for `anasum` and `effective area` code.
  - introduced consistent naming of tree reader support classes. Moved them from `.C` to `.cpp`. Removed obsolete code in these files.
  - removed `Version` from `CData` class initialize (as it is read from tree doc string).
  - removal of `CEffFit.h` classes (not used anywhere)
  - bug fix in SizeSecondMax calculation for 3-tel reconstruction using in `mscw_energy` the `rec_id` option (no image selection was applied)
  - maintenance and cleanup of obsolete files (see PR #294)

  ([#303](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/303))
- - minor updates to run with root 6.38.
  - switch default ROOT docker image to ubuntu

  ([#323](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/323))
- Change location of ROOT macros from ./macros/VTS to ./macros. ([#330](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/330))
- Removed deprecated smoothing and interpolation functionality for MVA cut values.
  Cleaned up unused parameters and obsolete code related to box smoothing and TMVA optimization. ([#336](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/336))

- Update URL for SOFA download. ([#317](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/317))
- Update SOFA to version 2023-10-11. ([#284](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/284))
- Changes in tree structure and variable types to reduce size of output trees (mostly due to change of doubles to floats; removal of unused variables); reduction is on storage requirements is ~25%. ([#288](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/288))
- Replaced many ROOT auto-generated header/source files (e.g., CData.h, CEffArea.h, CRunSummary.h, Cshowerpars.h, Ctpars.h, Ctelconfig.h) with hand-edited or cleaner C++ code, improving maintainability and readability. Moved implementation code from `.C` files to `.cpp` files, and deleted the old `.C` files.
  Adjusted several functions and constructors to reflect changes in how data and reconstruction methods are handled (e.g., CData and related classes now use cleaner, more explicit constructors).
  Functions that previously accepted "version" or "short" flags via integers now use explicit booleans for clarity.
  Removed the CEffFit class and related files.
  Pruned unused variables and methods, including some internal variables in VTMVAEvaluator, VTableLookupDataHandler, and others.
  Removed outdated warnings, TODO comments, and deprecated features. ([#289](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/289))
- Introduce towncrier for changelogs. ([#300](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/300))
- Fix github action worklows to compile Eventdisplay. Add cpp11 workflow. ([#307](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/307))
- Performance improvements for mscw energy stage. ([#312](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/312))

### Bugfixes

- Fix segmentation fault when exiting on error due to an empty tree in anasum. ([#286](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/286))
- Limit number of errors printed when reading faulty VBF files to avoid TB large log files. ([#298](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/298))
- Add cut to wobble offset 0.5 for calculation of MC-derived gamma-ray rates used for BDT cut optimization. ([#314](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/314))

### Documentation

- Add libtool compile instructions to readme. ([#287](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/287))
- Improved description of docker image building. ([#292](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/292))
