# Changelog

All notable changes to the Eventdisplay_v4 project will be documented in this file.
Changes for upcoming releases can be found in the [docs/changes](docs/changes) directory.
Note that changes before release v492.0 are not documented here, but can be found in the
[GitHub repository](https://github.com/VERITAS-Observatory/VTS-SimPipe/releases).

This changelog is generated using [Towncrier](https://towncrier.readthedocs.io/).

<!-- towncrier release notes start -->

## [v492.0](https://github.com/VERITAS-Observatory/EventDisplay_v4/releases/tag/v492.0) - 2025-10-26

### Bugfixes

- Fix segmentation fault when exiting on error due to an empty tree in anasum. ([#286](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/286))
- Limit number of errors printed when reading faulty VBF files to avoid TB large log files. ([#298](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/298))

### Documentation

- Add libtool compile instructions to readme. ([#287](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/287))
- Improved description of docker image building. ([#292](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/292))

### New Feature

- Introduction of average zenith angle calculation per run in VTMVAEvaluator (fAverageZenithPerRun, calculate_average_zenith_angle()). ([#281](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/281))
- Introduced new functionality for reconstructing events using any three telescopes out of four (primarily for Monte Carlo data and effective area calculations).
  Added new class: `VMeanScaledVariables` for mean scaled variable calculations.
  Updated VDispAnalyzer and VSimpleStereoReconstructor to support new reconstruction modes and calculation methods.
  Updated VInstrumentResponseFunctionRunParameter and related code to include new parameters for 3-telescope reconstruction. ([#289](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/289))
- Add new command line parameter `--minpedestalevents` to set the minimum number of required pedestal events (default: 50). ([#298](https://github.com/VERITAS-Observatory/EventDisplay_v4/issues/298))

### Maintenance

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
