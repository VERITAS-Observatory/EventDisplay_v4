#########################################################################
# Makefile for eventdisplay analysis package
##########################################################################
#
#  for VERITAS: make VTS
#
#  shell variables needed:
#    ROOTSYS (pointing to your root installation)
#
#  Libraries needed either for VTS:
#
#  for reading of VBF files (optional, VERITAS only)
#    VBFSYS  (pointing to VBF installation)
#    or
#   vbfConfig exists
#
#  Optional libraries:
#
#  for using GSL libraries (optional)
#    GSLSYS  (pointing to GSL installation)
#    or
#    gsl-config exists
#
#  for using FITS (optional)
#    FITSSYS (pointing to FITS installation)
#
#   for using sofa (default)
#      SOFASYS
#
##########################################################################
SHELL = /bin/sh
ARCH = $(shell uname)

#############################
# basic numbers
#############################
package = EVNDISP
version = 490
#############################
#############################
# check root version number
#############################
ROOTVERSION=$(shell root-config --version)
# check if this is root 6 (or later)
ROOT6=$(shell expr 5.99 \>= `root-config --version | cut -f1 -d \/`)
ROOT_CntCln = rootcling
#############################
# check for root libraries
#############################
# mysql support
# (necessary for VERITAS data analysis)
ROOT_MYSQL=$(shell root-config --has-mysql)
#############################
# VERITAS BANK FORMAT (VBF)
#############################
# check if for envirnomental variable $VBFSYS
ifeq ($(origin VBFSYS), undefined)
# check the default directory
  VBFTEST=$(wildcard /usr/local/bin/vbfConfig)
  ifeq ($(strip $(VBFTEST)),)
   VBFFLAG=-DNOVBF
  endif
endif
#############################
# VERITAS DATABASE
# (necessary for VERITAS data analysis)
#############################
# check that root is compiled with mysql
DBTEST=$(shell root-config --has-mysql)
ifeq ($(DBTEST),yes)
  DBFLAG=-DRUNWITHDB
endif
# DBFLAG=""
#####################
# GSL libraries
#####################
ifeq ($(origin GSLSYS), undefined)
# test if gsl-config exists
  GSLTEST=$(shell which gsl-config)
  ifeq ($(strip $(GSLTEST)),)
    GSLFLAG=-DNOGSL
  endif
  ifeq ($(strip $(GSLTEST)),"")
    GSLFLAG=-DNOGSL
  endif
endif
#####################
# FITS ROUTINES
# (optional, necessary for root to FITS converter)
#####################
ifeq ($(strip $(FITSSYS)),)
FITS = FALSE
endif

#####################
# ASTRONMETRY ROUTINES
#
ASTRONMETRY = -DASTROSOFA
ifeq ($(strip $(SOFASYS)),)
ASTRONMETRY = -DASTROSLALIB
endif
########################################################################################################################
# compile and linker flags
########################################################################################################################
# compiler and linker general values
CXX           = g++
CXXFLAGS      = -O3 -g -Wall -fPIC -fno-strict-aliasing  -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_SOURCE -D_LARGEFILE64_SOURCE
CXXFLAGS     += -I. -I./inc/
CXXFLAGS     += $(VBFFLAG) $(DBFLAG) $(GSLFLAG) $(ASTRONMETRY)
LD            = g++
OutPutOpt     = -o
INCLUDEFLAGS  = -I. -I./inc/

# linux depending flags
ifeq ($(ARCH),Linux)
	LDFLAGS       = -O
	SOFLAGS       = -shared
endif
# Apple OS X flags
ifeq ($(ARCH),Darwin)
CXX           = clang++
LD            = clang++
CXXFLAGS    += -Wdeprecated-declarations -stdlib=libc++ -std=c++11
LDFLAGS       = -bind_at_load
DllSuf        = dylib
UNDEFOPT      = dynamic_lookup
SOFLAGS       = -dynamiclib -single_module -undefined $(UNDEFOPT)
endif
# check compiler
GCCVERSION=$(shell $(CXX) -dumpversion)
GCCMACHINE=$(shell $(CXX) -dumpmachine)
# get major version of gcc, e.g. '4' in '4.6.'
GCC_VER_MAJOR := $(shell echo $(GCCVERSION) | cut -f1 -d.)
# get minor version of gcc, e.g. '6' in '4.6'
GCC_VER_MINOR := $(shell echo $(GCCVERSION) | cut -f2 -d.)
# check if gcc version is smaller than 4.8.
GCC_GT_4_8 := $(shell [ $(GCC_VER_MAJOR) -lt 3 -o \( $(GCC_VER_MAJOR) -eq 4 -a $(GCC_VER_MINOR) -lt 8 \) ] && echo true)
# check if gcc version is smaller than 4.9.
GCC_GT_4_9 := $(shell [ $(GCC_VER_MAJOR) -lt 3 -o \( $(GCC_VER_MAJOR) -eq 4 -a $(GCC_VER_MINOR) -lt 9 \) ] && echo true)
########################################################
# CXX FLAGS (taken from root)
########################################################
ifeq ($(GCC_GT_4_9),true)
   ROOTCFLAGS 	= -pthread -m64 -std=c++11
else
   ROOTCFLAGS   = $(shell root-config --auxcflags)
endif
CXXFLAGS     += $(ROOTCFLAGS)
CXXFLAGS     += -I$(shell root-config --incdir) -I$(shell root-config --incdir)/TMVA
########################################################
# root libs
########################################################
ROOTGLIBS     = $(shell root-config --glibs)
GLIBS         = $(ROOTGLIBS)
GLIBS        += -lMLP -lTreePlayer -lTMVA -lMinuit -lXMLIO -lSpectrum
########################################################
# VBF
########################################################
ifneq ($(VBFFLAG),-DNOVBF)
VBFCFLAGS     = -I$(VBFSYS)/include/VBF/
VBFLIBS       = $(shell $(VBFSYS)/bin/vbfConfig --ldflags --libs)
CXXFLAGS     += $(VBFCFLAGS)
endif
########################################################
# GSL FLAGS
########################################################
# GSLFLAG=-DNOGSL
ifneq ($(GSLFLAG),-DNOGSL)
GSLCFLAGS    = $(shell gsl-config --cflags)
GSLLIBS      = $(shell gsl-config --libs)
GLIBS        += $(GSLLIBS)
CXXFLAGS     += $(GSLCFLAGS)
endif
########################################################
# FITS
########################################################
ifneq ($(FITS),FALSE)
GLIBS		+= -L$(FITSSYS)/lib -lcfitsio
CXXFLAGS	+= -I$(FITSSYS)/include/
endif
########################################################
# ASTROMETRY
########################################################
ifeq ($(ASTRONMETRY),-DASTROSOFA)
GLIBS		+= -L$(SOFASYS)/lib -lsofa_c
CXXFLAGS	+= -I$(SOFASYS)/include/
endif
########################################################

########################################################
# paths
########################################################
VPATH = src:inc

########################################################
# compilation and linking
#
# binaries: ./bin directory
# libraries: ./lib directory
########################################################

all VTS:	evndisp \
        printRunParameter \
		printAnasumRunParameter \
	mscw_energy \
	anasum makeDISPTables \
	combineDISPTables \
	printDISPTables \
	combineLookupTables \
	makeEffectiveArea \
	trainTMVAforGammaHadronSeparation \
	VTS.calculateCrabRateFromMC \
	VTS.calculateExposureFromDB \
	slib \
	combineEffectiveAreas \
	makeRadialAcceptance \
	calculateBinaryPhases \
	compareDatawithMC \
	VTS.getRunListFromDB \
	VTS.getLaserRunFromDB \
	VTS.getRun_TimeElevAzim \
	writeParticleRateFilesForTMVA \
	writelaserinDB \
	logFile

###############################################################################################################################
# core eventdisplay package
###############################################################################################################################


########################################################
# eventdisplay
########################################################
EVNOBJECTS =    ./obj/VVirtualDataReader.o \
		./obj/VGrIsuReader.o \
		./obj/VMultipleGrIsuReader.o \
		./obj/VDSTReader.o \
		./obj/VNoiseFileReader.o \
         ./obj/VCamera.o \
		./obj/VDisplayBirdsEye.o \
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VCameraRead.o \
		./obj/VDetectorGeometry.o \
		./obj/VDetectorTree.o \
	    ./obj/VImageParameterCalculation.o \
		./obj/VImageBaseAnalyzer.o \
		./obj/VImageCleaning.o \
		./obj/VDB_CalibrationInfo.o\
		./obj/VDB_Connection.o\
		./obj/VEvndispRunParameter.o  ./obj/VEvndispRunParameter_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VReadRunParameter.o \
		./obj/VEvndispData.o \
		./obj/VImageAnalyzerData.o \
		./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VCalibrationData.o \
		./obj/VCalibrator.o \
        ./obj/VImageAnalyzer.o \
		./obj/VArrayAnalyzer.o \
		./obj/VMLPAnalyzer.o \
		./obj/VDispAnalyzer.o \
		./obj/VDispTableReader.o \
		./obj/VDispTableReader_Dict.o \
		./obj/VDispTableAnalyzer.o \
		./obj/VTMVADispAnalyzer.o \
		./obj/VShowerParameters.o \
		./obj/VMCParameters.o \
		./obj/VGrIsuAnalyzer.o \
		./obj/VImageParameter.o \
		./obj/VTraceHandler.o \
		./obj/VFitTraceHandler.o \
		./obj/VImageAnalyzerHistograms.o \
		./obj/VDST.o \
		./obj/VDSTTree.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
		./obj/VPedestalCalculator.o \
		./obj/VDeadChannelFinder.o \
		./obj/VSpecialChannel.o \
		./obj/VDeadTime.o \
		./obj/VEventLoop.o \
		./obj/VDBRunInfo.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VUtilities.o \
		./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
		./obj/VPointing.o \
	 	./obj/VPointingDB.o \
		./obj/VSkyCoordinates.o \
		./obj/VArrayPointing.o \
		./obj/VStarCatalogue.o  ./obj/VStarCatalogue_Dict.o \
		./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/VTrackingCorrections.o \
		./obj/CorrectionParameters.o \
		./obj/Angle.o \
		./obj/PointingMonitor.o \
		./obj/VSkyCoordinatesUtilities.o \
		./obj/VHoughTransform.o \
		./obj/VDB_PixelDataReader.o \
		./obj/VDisplay.o \
		./obj/VDeadPixelOrganizer.o


ifneq ($(ARCH),Darwin)
EVNOBJECTS += ./obj/VDisplay_Dict.o
endif

# add VBF objects
ifneq ($(VBFFLAG),-DNOVBF)
   EVNOBJECTS +=    ./obj/VRawDataReader.o \
		    ./obj/VBaseRawDataReader.o  \
		    ./obj/VBFDataReader.o \
	 	    ./obj/VSimulationDataReader.o
endif
# finalize
EVNOBJECTS += ./obj/evndisp.o

# compile and link
./obj/evndisp.o:	./src/evndisp.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

evndisp:	$(EVNOBJECTS)
ifeq ($(GSLFLAG),-DNOGSL)
	@echo "LINKING evndisp without GSL libraries"
else
	@echo "LINKING evndisp with GSL libraries"
endif
ifeq ($(VBFFLAG),-DNOVBF)
	@echo "LINKING evndisp without VBF support"
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
else
	@echo "LINKING evndisp with VBF support"
	$(LD) $(LDFLAGS) $^ $(VBFLIBS) $(GLIBS) $(OutPutOpt) ./bin/$@
endif
	@echo "$@ done"

########################################################
# lookup table code (mscw_energy)
########################################################
MSCOBJECTS=	./obj/Cshowerpars.o ./obj/Ctpars.o \
                ./obj/Ctelconfig.o ./obj/VTableLookupDataHandler.o ./obj/VTableCalculator.o \
		./obj/VTableEnergyCalculator.o ./obj/VTableLookup.o ./obj/VTablesToRead.o \
		./obj/VEmissionHeightCalculator.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
		./obj/VDeadTime.o ./obj/VUtilities.o \
		./obj/VStatistics_Dict.o \
		./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
        ./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
        ./obj/VStar.o ./obj/VStar_Dict.o \
        ./obj/VUtilities.o \
        ./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
		./obj/VMedianCalculator.o \
        ./obj/VSkyCoordinatesUtilities.o \
        ./obj/VDB_Connection.o \
		./obj/mscw_energy.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    MSCOBJECTS += ./obj/VASlalib.o
endif

./obj/mscw_energy.o:	./src/mscw_energy.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

mscw_energy:  $(MSCOBJECTS)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# makeRadialAcceptance
########################################################

ACCOBJECT = 	./obj/makeRadialAcceptance.o \
		./obj/VRadialAcceptance.o \
		./obj/VSkyCoordinatesUtilities.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
		./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o ./obj/CData.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
		./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
		./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
		./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
		./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		./obj/VRunList.o ./obj/VRunList_Dict.o \
		./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VAnaSumRunParameter.o ./obj/VAnaSumRunParameter_Dict.o \
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		 ./obj/VUtilities.o


ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    ACCOBJECT += ./obj/VASlalib.o
endif


./obj/makeRadialAcceptance.o:	./src/makeRadialAcceptance.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

makeRadialAcceptance:	$(ACCOBJECT)
	$(LD) $(LDFLAGS) $^ $(GLIBS)  $(OutPutOpt) ./bin/$@
	@echo "$@ done"

##########################
# VTS.getRun_TimeElevAzim
##########################

ACCOBJECT = ./obj/VTS.getRun_TimeElevAzim.o \
		./obj/VSkyCoordinates.o \
		./obj/VSkyCoordinatesUtilities.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
		./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o ./obj/CData.o \
		./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
		./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
		./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
		./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		./obj/VRunList.o ./obj/VRunList_Dict.o \
		./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VAnaSumRunParameter.o ./obj/VAnaSumRunParameter_Dict.o \
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
		./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/VDB_Connection.o \
		./obj/VUtilities.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    VTSRUNTIMEOBJ += ./obj/VASlalib.o
endif

./obj/VTS.getRun_TimeElevAzim.o: ./src/VTS.getRun_TimeElevAzim.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTS.getRun_TimeElevAzim: $(ACCOBJECT)
	$(LD) $(LDFLAGS) $^ $(GLIBS)  $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# effective area code (makeEffectiveArea_
########################################################

EFFOBJECT =	./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o ./obj/CData.o ./obj/VEffectiveAreaCalculator.o \
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
		./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		./obj/VRunList.o ./obj/VRunList_Dict.o ./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VInstrumentResponseFunction.o ./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o ./obj/Ctelconfig.o \
		./obj/VInstrumentResponseFunctionRunParameter.o ./obj/VInstrumentResponseFunctionRunParameter_Dict.o \
		./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
		./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
		./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VSkyCoordinatesUtilities.o ./obj/VUtilities.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
		./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
		./obj/makeEffectiveArea.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    EFFOBJECT += ./obj/VASlalib.o
endif

./obj/makeEffectiveArea.o:	./src/makeEffectiveArea.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

makeEffectiveArea:	$(EFFOBJECT) ./obj/makeEffectiveArea.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# logFile
########################################################
LOGFILE =		./obj/logFile.o \
					

./obj/logFile.o:	./src/logFile.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

logFile:	$(LOGFILE)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# merge VBF files
########################################################
VBFMERGE=	./obj/mergeVBF.o

./obj/mergeVBF.o:    ./src/mergeVBF.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

mergeVBF: $(VBFMERGE)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(VBFLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# split VBF files
########################################################
VBFSPLIT=	./obj/splitVBF.o

./obj/splitVBF.o:    ./src/splitVBF.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

splitVBF: $(VBFSPLIT)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(VBFLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# anasum
########################################################
ANASUMOBJECTS =	./obj/VAnaSum.o ./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o ./obj/CData.o \
                ./obj/VStereoHistograms.o \
		./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
                ./obj/VStereoAnalysis.o \
                ./obj/VSkyCoordinates.o \
                ./obj/VOnOff.o ./obj/VAnaSumRunParameter.o ./obj/VAnaSumRunParameter_Dict.o \
		./obj/VStereoMaps.o ./obj/VRatePlots.o \
		./obj/VRadialAcceptance.o ./obj/VEffectiveAreaCalculator.o ./obj/VRunSummary.o \
		./obj/VDeadTime.o \
		./obj/VTimeMask.o ./obj/VTimeMask_Dict.o ./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		./obj/VRunList.o ./obj/VRunList_Dict.o \
		./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VInstrumentResponseFunctionRunParameter.o ./obj/VInstrumentResponseFunctionRunParameter_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o ./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
		./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o  \
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
		./obj/VTMVARunData.o ./obj/VTMVARunData_Dict.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/Ctelconfig.o ./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
		./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o ./obj/VTableLookupRunParameter.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VTableLookupRunParameter_Dict.o \
		./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
		./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
		./obj/VDB_Connection.o \
		./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VSkyCoordinatesUtilities.o ./obj/VUtilities.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/anasum.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    ANASUMOBJECTS += ./obj/VASlalib.o
endif

./obj/anasum.o:	./src/anasum.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

anasum:	$(ANASUMOBJECTS)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# shared library for root analysis
########################################################

SHAREDOBJS= 	./obj/VRunList.o ./obj/VRunList_Dict.o \
		./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
		./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VDB_Connection.o \
		./obj/CData.o \
		./obj/VAnalysisUtilities_Dict.o ./obj/VAnalysisUtilities.o \
		./obj/VPlotLookupTable.o ./obj/VPlotLookupTable_Dict.o \
		./obj/CEffArea.o ./obj/CEffArea_Dict.o \
		./obj/VFluxCalculation.o ./obj/VFluxCalculation_Dict.o \
		./obj/VStatistics_Dict.o \
		./obj/VDifferentialFlux.o ./obj/VDifferentialFlux_Dict.o \
		./obj/VSpectralFitter.o ./obj/VSpectralFitter_Dict.o \
		./obj/VEnergyThreshold.o ./obj/VEnergyThreshold_Dict.o \
		./obj/VEnergySpectrum.o ./obj/VEnergySpectrum_Dict.o \
		./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
		./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/Ctelconfig.o \
		./obj/VSkyCoordinatesUtilities.o ./obj/VSkyCoordinatesUtilities_Dict.o \
		./obj/VSkyCoordinates.o ./obj/VSkyCoordinates_Dict.o \
		./obj/VPlotRunSummary.o ./obj/VPlotRunSummary_Dict.o \
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VStereoReconstruction.o ./obj/VStereoReconstruction_Dict.o \
		./obj/VRunStats.o ./obj/VRunStats_Dict.o \
		./obj/VExposure.o ./obj/VExposure_Dict.o \
		./obj/VMonteCarloRateCalculator.o ./obj/VMonteCarloRateCalculator_Dict.o \
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		./obj/VAnaSumRunParameter.o ./obj/VAnaSumRunParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
		./obj/VInterpolate2DHistos.o ./obj/VInterpolate2DHistos_Dict.o \
		./obj/VSpectralEnergyDistribution.o ./obj/VSpectralEnergyDistribution_Dict.o \
		./obj/VPlotArrayReconstruction.o ./obj/VPlotArrayReconstruction_Dict.o \
		./obj/VSensitivityCalculator.o ./obj/VSensitivityCalculator_Dict.o \
		./obj/VDispTableReader.o ./obj/VDispTableReader_Dict.o \
		./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o \
		./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
		./obj/VDetectorGeometry.o ./obj/VDetectorGeometry_Dict.o \
		./obj/VCameraRead.o ./obj/VCameraRead_Dict.o \
		./obj/VDetectorTree.o ./obj/VDetectorTree_Dict.o \
		./obj/VInstrumentResponseFunctionReader.o ./obj/VInstrumentResponseFunctionReader_Dict.o \
		./obj/VPlotInstrumentResponseFunction.o ./obj/VPlotInstrumentResponseFunction_Dict.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VAtmosphereSoundings.o ./obj/VAtmosphereSoundings_Dict.o\
		./obj/VAtmosphereSoundingData.o ./obj/VAtmosphereSoundingData_Dict.o \
		./obj/VPlotSensitivityfromLisFiles.o ./obj/VPlotSensitivityfromLisFiles_Dict.o \
		./obj/VPlotMonteCarloQualityFactor.o ./obj/VPlotMonteCarloQualityFactor_Dict.o \
		./obj/VPlotAnasumHistograms.o ./obj/VPlotAnasumHistograms_Dict.o \
		./obj/VPlotCompareDataWithMC.o ./obj/VPlotCompareDataWithMC_Dict.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VDBTools.o ./obj/VDBTools_Dict.o \
		./obj/VTMVARunData.o ./obj/VTMVARunData_Dict.o \
		./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
		./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
		./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
		./obj/VInstrumentResponseFunctionRunParameter.o ./obj/VInstrumentResponseFunctionRunParameter_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VLightCurve.o ./obj/VLightCurve_Dict.o \
		./obj/VLightCurveData.o ./obj/VLightCurveData_Dict.o \
		./obj/VLightCurveUtilities.o ./obj/VLightCurveUtilities_Dict.o \
		./obj/VLombScargle.o ./obj/VLombScargle_Dict.o \
		./obj/VZDCF.o ./obj/VZDCF_Dict.o ./obj/VZDCFData.o \
		./obj/VUtilities.o \
		./obj/VPlotRadialAcceptance.o ./obj/VPlotRadialAcceptance_Dict.o \
		./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
		./obj/Cshowerpars.o \
		./obj/Ctpars.o \
		./obj/VPlotEvndispReconstructionParameter.o ./obj/VPlotEvndispReconstructionParameter_Dict.o \
		./obj/VImageParameter.o  \
		./obj/VPlotWPPhysSensitivity.o ./obj/VPlotWPPhysSensitivity_Dict.o \
		./obj/VSiteData.o \
		./obj/VPlotTMVAParameters.o ./obj/VPlotTMVAParameters_Dict.o \
		./obj/VWPPhysSensitivityPlotsMaker.o ./obj/VWPPhysSensitivityPlotsMaker_Dict.o \
		./obj/VPedestalLowGain.o ./obj/VPedestalLowGain_Dict.o \
		./obj/VLowGainCalibrator.o ./obj/VLowGainCalibrator_Dict.o \
		./obj/VTimeMask.o ./obj/VTimeMask_Dict.o \
		./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
		./obj/VPlotOptimalCut.o ./obj/VPlotOptimalCut_Dict.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
  SHAREDOBJS += ./obj/VASlalib.o ./obj/VASlalib_Dict.o 
endif

ifneq ($(GSLFLAG),-DNOGSL)
  SHAREDOBJS	+= ./obj/VLikelihoodFitter.o ./obj/VLikelihoodFitter_Dict.o
endif

ifneq ($(FITS),FALSE)
  SHAREDOBJS	+= ./obj/VFITS.o  ./obj/VFITS_Dict.o
endif

slib lsib ./lib/libVAnaSum.so:   $(SHAREDOBJS)
	mkdir -p ./lib
	$(LD) $(SOFLAGS) $(SHAREDOBJS) $(GLIBS) $(OutPutOpt) ./lib/libVAnaSum.so
	@echo "COPYING pcm FILES TO lib"
	cp -f ./obj/*.pcm ./lib/
	cp -f ./obj/*.pcm ./bin/

########################################################
# printAnasumRunParameter
########################################################
PRINTANAOBJ=	./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
                ./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
		./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o \
		./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
		./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VUtilities.o \
		./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
		./obj/VSkyCoordinatesUtilities.o \
		./obj/VTimeMask.o ./obj/VTimeMask_Dict.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
		./obj/VRunList.o ./obj/VRunList_Dict.o \
		./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
		./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
		./obj/VAnaSumRunParameter.o ./obj/VAnaSumRunParameter_Dict.o \
		./obj/printAnasumRunParameter.o

./obj/printAnasumRunParameter.o:	./src/printAnasumRunParameter.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

printAnasumRunParameter:	$(PRINTANAOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# printRunParameter
########################################################
PRINTRUNOBJ=	./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VUtilities.o ./obj/VTableLookupRunParameter.o \
		./obj/VTableLookupRunParameter_Dict.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
        ./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
        ./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
        ./obj/VSkyCoordinatesUtilities.o \
        ./obj/VDB_Connection.o \
		./obj/printRunParameter.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    PRINTRUNOBJ += ./obj/VASlalib.o
endif

./obj/printRunParameter.o:	./src/printRunParameter.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

printRunParameter:	$(PRINTRUNOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# makeDISPTables
########################################################
MAKEDISPTABLESOBJ=	./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
			./obj/VDispTable.o ./obj/VDispTableReader.o ./obj/VDispTableReader_Dict.o \
			./obj/Cshowerpars.o ./obj/Ctpars.o \
			./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
                        ./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
                        ./obj/VStar.o ./obj/VStar_Dict.o \
			./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
                        ./obj/VSkyCoordinatesUtilities.o \
                        ./obj/VDB_Connection.o \
			./obj/VUtilities.o \
			./obj/makeDISPTables.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    MAKEDISPTABLESOBJ += ./obj/VASlalib.o
endif

./obj/makeDISPTables.o:	./src/makeDISPTables.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

makeDISPTables:	$(MAKEDISPTABLESOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# combineDISPTables
########################################################
COMBINEDISPTABLESOBJ=	./obj/VDispTable.o \
			./obj/VDispTableReader.o ./obj/VDispTableReader_Dict.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/Cshowerpars.o ./obj/Ctpars.o \
			./obj/combineDISPTables.o

./obj/combineDISPTables.o:	./src/combineDISPTables.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

combineDISPTables:	$(COMBINEDISPTABLESOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# printDISPTables
########################################################
PRINTDISPTABLESOBJ= 	./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			./obj/VUtilities.o \
                        ./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
                        ./obj/VStar.o ./obj/VStar_Dict.o \
			./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
                        ./obj/VSkyCoordinatesUtilities.o \
                        ./obj/VDB_Connection.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
			./obj/VDispTable.o ./obj/VDispTableReader.o ./obj/VDispTableReader_Dict.o \
			./obj/Cshowerpars.o ./obj/Ctpars.o \
			./obj/printDISPTables.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    PRINTDISPTABLESOBJ += ./obj/VASlalib.o
endif

./obj/printDISPTables.o:	./src/printDISPTables.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

printDISPTables:	$(PRINTDISPTABLESOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# compareDatawithMC
########################################################
COMPAREDATAMCOBJ=	./obj/CData.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
			./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
			./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
			./obj/VSkyCoordinates.o \
			./obj/VSkyCoordinatesUtilities.o \
			./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
			./obj/VDB_Connection.o \
		   	./obj/VStar.o ./obj/VStar_Dict.o \
			./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
			./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
			./obj/VUtilities.o \
			./obj/VDataMCComparision.o \
			./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o \
			./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
			./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		     	./obj/VInstrumentResponseFunction.o \
		     	./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
			./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
			./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
			./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
			./obj/VRunList.o ./obj/VRunList_Dict.o ./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
			./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o ./obj/Ctelconfig.o \
			./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
			./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
			./obj/compareDatawithMC.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    COMPAREDATAMCOBJ += ./obj/VASlalib.o
endif

./obj/compareDatawithMC.o:	./src/compareDatawithMC.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

compareDatawithMC:	$(COMPAREDATAMCOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# printBinaryOrbitalPhase
########################################################
PRINTBINARYOBJ=		./obj/VAstronometry.o ./obj/printBinaryOrbitalPhase.o
ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    PRINTBINARYOBJ += ./obj/VASlalib.o
endif

./obj/printBinaryOrbitalPhase.o:	./src/printBinaryOrbitalPhase.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

printBinaryOrbitalPhase:	$(PRINTBINARYOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# testAstronometry
########################################################
TESTASTROMETRYOBJ =		./obj/VAstronometry.o ./obj/testAstronometry.o
ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    TESTASTROMETRYOBJ += ./obj/VASlalib.o
endif

./obj/testAstronometry.o:	./src/testAstronometry.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

testAstronometry:	$(TESTASTROMETRYOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# writeVTSWPPhysSensitivityFiles
########################################################
WRITEVTSPHYSOBJ=	./obj/VWPPhysSensitivityFile.o \
			./obj/writeVTSWPPhysSensitivityFiles.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
			./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
			./obj/VInstrumentResponseFunctionReader.o ./obj/VInstrumentResponseFunctionReader_Dict.o \
			./obj/VInstrumentResponseFunctionRunParameter.o ./obj/VInstrumentResponseFunctionRunParameter_Dict.o \
			./obj/VSensitivityCalculator.o ./obj/VSensitivityCalculator_Dict.o \
			./obj/CEffArea.o ./obj/CEffArea_Dict.o \
			./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
			./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
			./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
			./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
			./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o \
			./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
			./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
			./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
			./obj/VSpectralFitter.o ./obj/VSpectralFitter_Dict.o \
			./obj/VEnergyThreshold.o ./obj/VEnergyThreshold_Dict.o \
			./obj/VRunList.o ./obj/VRunList_Dict.o \
			./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
			./obj/VEnergySpectrum.o ./obj/VEnergySpectrum_Dict.o \
			./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o  \
			./obj/VDifferentialFlux.o ./obj/VDifferentialFlux_Dict.o \
			./obj/VMonteCarloRateCalculator.o ./obj/VMonteCarloRateCalculator_Dict.o \
			./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
			./obj/VEvndispRunParameter.o  ./obj/VEvndispRunParameter_Dict.o \
			./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			./obj/VSkyCoordinatesUtilities.o \
			./obj/VTimeMask.o ./obj/VTimeMask_Dict.o \
			./obj/VStatistics_Dict.o \
			./obj/VUtilities.o \
			./obj/Ctelconfig.o

ifneq ($(GSLFLAG),-DNOGSL)
  WRITEVTSPHYSOBJ	+= ./obj/VLikelihoodFitter.o ./obj/VLikelihoodFitter_Dict.o
endif

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    WRITEVTSPHYSOBJ += ./obj/VASlalib.o
endif

./obj/writeVTSWPPhysSensitivityFiles.o: 	./src/writeVTSWPPhysSensitivityFiles.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

writeVTSWPPhysSensitivityFiles:	$(WRITEVTSPHYSOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# writeParticleRateFilesFromEffectiveAreas
########################################################
WRITEPARTPHYSOBJ=	./obj/writeParticleRateFilesFromEffectiveAreas.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
			./obj/VASlalib.o \
			./obj/VInstrumentResponseFunctionReader.o ./obj/VInstrumentResponseFunctionReader_Dict.o \
			./obj/VSensitivityCalculator.o ./obj/VSensitivityCalculator_Dict.o \
			./obj/CEffArea.o ./obj/CEffArea_Dict.o \
			./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
			./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
			./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
			./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
			./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o \
			./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
			./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
			./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
			./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
			./obj/VInstrumentResponseFunctionRunParameter.o ./obj/VInstrumentResponseFunctionRunParameter_Dict.o \
			./obj/Ctelconfig.o  \
			./obj/VSpectralFitter.o ./obj/VSpectralFitter_Dict.o \
			./obj/VEnergyThreshold.o ./obj/VEnergyThreshold_Dict.o \
			./obj/VRunList.o ./obj/VRunList_Dict.o \
			./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
			./obj/VEnergySpectrum.o ./obj/VEnergySpectrum_Dict.o \
			./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o  \
			./obj/VDifferentialFlux.o ./obj/VDifferentialFlux_Dict.o \
			./obj/VMonteCarloRateCalculator.o ./obj/VMonteCarloRateCalculator_Dict.o \
			./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
			./obj/VStatistics_Dict.o \
			./obj/VUtilities.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    WRITECTAPHYSOBJ += ./obj/VASlalib.o
endif

./obj/writeParticleRateFilesFromEffectiveAreas.o: 	./src/writeParticleRateFilesFromEffectiveAreas.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

writeParticleRateFilesFromEffectiveAreas:	$(WRITEPARTPHYSOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# writeParticleRateFilesForTMVA
########################################################
WRITEPARTPHYSOBJ=	./obj/writeParticleRateFilesForTMVA.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
			./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
			./obj/VInstrumentResponseFunctionReader.o ./obj/VInstrumentResponseFunctionReader_Dict.o \
			./obj/VSensitivityCalculator.o ./obj/VSensitivityCalculator_Dict.o \
			./obj/CEffArea.o ./obj/CEffArea_Dict.o \
			./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
			./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
			./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
			./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
			./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o \
			./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
			./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
			./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
			./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
			./obj/VInstrumentResponseFunctionRunParameter.o ./obj/VInstrumentResponseFunctionRunParameter_Dict.o \
			./obj/Ctelconfig.o  \
			./obj/VSpectralFitter.o ./obj/VSpectralFitter_Dict.o \
			./obj/VEnergyThreshold.o ./obj/VEnergyThreshold_Dict.o \
			./obj/VRunList.o ./obj/VRunList_Dict.o \
			./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
			./obj/VEnergySpectrum.o ./obj/VEnergySpectrum_Dict.o \
			./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o  \
			./obj/VDifferentialFlux.o ./obj/VDifferentialFlux_Dict.o \
			./obj/VMonteCarloRateCalculator.o ./obj/VMonteCarloRateCalculator_Dict.o \
			./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
			./obj/VStatistics_Dict.o \
			./obj/VUtilities.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    WRITECTAPHYSOBJ += ./obj/VASlalib.o
endif

./obj/writeParticleRateFilesForTMVA.o: 	./src/writeParticleRateFilesForTMVA.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

writeParticleRateFilesForTMVA:	$(WRITEPARTPHYSOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# combineLookupTables
########################################################
./obj/combineLookupTables.o:	./src/combineLookupTables.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

combineLookupTables:	./obj/combineLookupTables.o ./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# checkAnalysisResultFile
########################################################

./obj/checkAnalysisResultFile.o:	./src/checkAnalysisResultFile.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

checkAnalysisResultFile:	./obj/checkAnalysisResultFile.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# trainTMVAforAngularReconstruction
########################################################
./obj/trainTMVAforAngularReconstruction.o:	./src/trainTMVAforAngularReconstruction.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

trainTMVAforAngularReconstruction:	./obj/trainTMVAforAngularReconstruction.o \
					./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
					./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
					./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
					./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
					./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
					./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
					./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
					./obj/VStar.o ./obj/VStar_Dict.o \
					./obj/VDB_Connection.o \
					./obj/VSkyCoordinatesUtilities.o \
					./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
					./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
					./obj/VUtilities.o \
					./obj/Ctelconfig.o ./obj/Cshowerpars.o ./obj/Ctpars.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"
########################################################
# updateDBlaserRUN
########################################################
./obj/updateDBlaserRUN.o: ./src/updateDBlaserRUN.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

updateDBlaserRUN:	./obj/VDBTools.o ./obj/VDBTools_Dict.o \
			./obj/VExposure.o ./obj/VExposure_Dict.o \
			./obj/VStar.o ./obj/VStar_Dict.o \
			./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
			./obj/VDB_Connection.o \
			./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
			./obj/VSkyCoordinatesUtilities.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VUtilities.o \
			./obj/VDB_CalibrationInfo.o \
			./obj/updateDBlaserRUN.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "Done updateDBlaserRUN"

########################################################
# updateDBlaserRUN
# ########################################################
writelaserinDBOBJ  = ./obj/VDB_CalibrationInfo.o
writelaserinDBOBJ += ./obj/VDB_Connection.o
writelaserinDBOBJ += ./obj/writelaserinDB.o

./obj/writelaserinDB.o : ./src/writelaserinDB.cpp
	$(CXX) $(CXXFLAGS) -Wno-write-strings -Wno-unused-function -c -o $@ $<

writelaserinDB : $(writelaserinDBOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# combineEffectiveAreas
########################################################

COMBINEEFFAREAOBJ=	 ./obj/combineEffectiveAreas.o  \
			 ./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			 ./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			 ./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
			 ./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
			 ./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
			 ./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
			 ./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
			 ./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
			 ./obj/VRunList.o ./obj/VRunList_Dict.o ./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
			 ./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
			 ./obj/VInstrumentResponseFunction.o ./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
			 ./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
			 ./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
			 ./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o ./obj/Ctelconfig.o \
			 ./obj/VInstrumentResponseFunctionRunParameter.o ./obj/VInstrumentResponseFunctionRunParameter_Dict.o \
			 ./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
			 ./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
			./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
			 ./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			 ./obj/VSkyCoordinatesUtilities.o ./obj/VUtilities.o \
			 ./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
			 ./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o \
			 ./obj/combineEffectiveAreas.o

./obj/combineEffectiveAreas.o:	./src/combineEffectiveAreas.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    COMBINEEFFOBJ += ./obj/VASlalib.o
endif

combineEffectiveAreas:	$(COMBINEEFFAREAOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"


########################################################
# trainTMVAforGammaHadronSeparation
########################################################
MAKEOPTCUTTMVAOBJ=	./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
			./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
			./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
			./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
			./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
			./obj/VTMVARunData.o ./obj/VTMVARunData_Dict.o \
			./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
			./obj/VTMVARunDataZenithCut.o ./obj/VTMVARunDataZenithCut_Dict.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VUtilities.o \
			./obj/trainTMVAforGammaHadronSeparation.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    MAKEOPTCUTTMVAOBJ += ./obj/VASlalib.o
endif

./obj/trainTMVAforGammaHadronSeparation.o:	./src/trainTMVAforGammaHadronSeparation.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

trainTMVAforGammaHadronSeparation:	$(MAKEOPTCUTTMVAOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "Done"

########################################################
# trainTMVAforGammaHadronSeparation_TrainingFile
########################################################
MAKEOPTCUTTMVATRAININGOBJ= 	./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
				./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
				./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
				./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
				./obj/VEvndispRunParameter.o obj/VEvndispRunParameter_Dict.o \
				./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
				./obj/trainTMVAforGammaHadronSeparation_TrainingFile.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    MAKEOPTCUTTMVATRAININGOBJ += ./obj/VASlalib.o
endif

./obj/trainTMVAforGammaHadronSeparation_TrainingFile.o:	./src/trainTMVAforGammaHadronSeparation_TrainingFile.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

trainTMVAforGammaHadronSeparation_TrainingFile:	$(MAKEOPTCUTTMVATRAININGOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "Done"

########################################################
# VTS.calculateCrabRateFromMC
########################################################
./obj/VTS.calculateCrabRateFromMC.o:	./src/VTS.calculateCrabRateFromMC.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTS.calculateCrabRateFromMC:	./obj/CEffArea.o ./obj/CEffArea_Dict.o \
				./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
				./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
				./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
				./obj/VRunList_Dict.o ./obj/VRunList.o \
				./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
				./obj/VMonteCarloRateCalculator.o ./obj/VMonteCarloRateCalculator_Dict.o \
				./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
                                ./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
                                ./obj/VStar.o ./obj/VStar_Dict.o \
                                ./obj/VUtilities.o \
                                 ./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
                                ./obj/VSkyCoordinatesUtilities.o \
                                ./obj/VDB_Connection.o \
				./obj/VTS.calculateCrabRateFromMC.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# VTS.analyzeMuonRings
########################################################
./obj/VTS.analyzeMuonRings.o:	./src/VTS.analyzeMuonRings.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTS.analyzeMuonRings:		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
				./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
				./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		                ./obj/VDB_Connection.o \
				./obj/Ctelconfig.o ./obj/Cshowerpars.o ./obj/Ctpars.o \
				./obj/VTS.analyzeMuonRings.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# VTS.calculateExposureFromDB
########################################################
./obj/VTS.calculateExposureFromDB.o:	./src/VTS.calculateExposureFromDB.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTS.calculateExposureFromDB:	./obj/VDBTools.o ./obj/VDBTools_Dict.o \
				./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
				./obj/VStar.o ./obj/VStar_Dict.o \
				./obj/VExposure.o ./obj/VExposure_Dict.o \
				./obj/VDB_Connection.o \
				./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
                ./obj/VUtilities.o \
				./obj/VUtilities.o \
				./obj/VSkyCoordinatesUtilities.o \
				./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
				./obj/VTS.calculateExposureFromDB.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# VTS.getLaserRunFromDB
########################################################
./obj/VTS.getLaserRunFromDB.o:   ./src/VTS.getLaserRunFromDB.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTSLASERUNOBJ=	./obj/VDBTools.o ./obj/VDBTools_Dict.o \
			./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
			./obj/VStar.o ./obj/VStar_Dict.o \
                        ./obj/VUtilities.o \
                        ./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
                        ./obj/VSkyCoordinatesUtilities.o \
			./obj/VDBRunInfo.o \
			./obj/VDB_Connection.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VTS.getLaserRunFromDB.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    VTSLASERUNOBJ += ./obj/VASlalib.o
endif

VTS.getLaserRunFromDB:	$(VTSLASERUNOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# VTS.getRunListFromDB
########################################################
./obj/VTS.getRunListFromDB.o:   ./src/VTS.getRunListFromDB.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTSRUNLISTDBOJB=	./obj/VDBTools.o ./obj/VDBTools_Dict.o \
			./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
			./obj/VStar.o ./obj/VStar_Dict.o \
			./obj/VExposure.o ./obj/VExposure_Dict.o \
			./obj/VDB_Connection.o \
			./obj/VAstronometry.o ./obj/VAstronometry_Dict.o \
			./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
			./obj/VStar.o ./obj/VStar_Dict.o \
			./obj/VUtilities.o \
			./obj/VSkyCoordinatesUtilities.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VUtilities.o \
			./obj/VTS.getRunListFromDB.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    VTSRUNLISTDBOJB += ./obj/VASlalib.o
endif

VTS.getRunListFromDB:	$(VTSRUNLISTDBOJB)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# calculateBinaryPhases
########################################################
./obj/calculateBinaryPhases.o:	./src/calculateBinaryPhases.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

calculateBinaryPhases:	./obj/CData.o \
			./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VOrbitalPhase.o ./obj/VOrbitalPhase_Dict.o \
			./obj/calculateBinaryPhases.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# next day analysis
########################################################

./obj/VTS.next_day.o:	./src/VTS.next_day.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTS.next_day:	./obj/VFITS.o ./obj/VFITS_Dict.o \
		./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
		./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
		./obj/VRunList_Dict.o ./obj/VRunList.o \
		./obj/VEnergySpectrum.o ./obj/VEnergySpectrum_Dict.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o  \
		./obj/VDifferentialFlux.o ./obj/VDifferentialFlux_Dict.o \
		./obj/VSpectralFitter.o ./obj/VSpectralFitter_Dict.o \
		./obj/VEnergyThreshold.o ./obj/VEnergyThreshold_Dict.o \
		./obj/CEffArea.o ./obj/CEffArea_Dict.o \
		./obj/VFluxCalculation.o ./obj/VFluxCalculation_Dict.o \
		./obj/VLightCurve.o ./obj/VLightCurve_Dict.o \
		./obj/VLightCurveData.o ./obj/VLightCurveData_Dict.o \
		./obj/VLightCurveUtilities.o ./obj/VLightCurveUtilities_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VTS.next_day.o
	$(LD) $(LDFLAGS) $^ $(GLIBS)  $(OutPutOpt) ./bin/$@
	@echo "$@ done"

TESTFITS:
ifeq ($(FITS),FALSE)
	   @echo ""
	   @echo "----------------------------------------"
	   @echo "NO FITSSYS ENVIRONMENTAL VARIABLE SET"
	   @echo "----------------------------------------"
	   @echo "";
endif

########################################################
# implicit rules
########################################################
./obj/%.o:	%.cpp %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

./obj/%.o:	%.C
	$(CXX) $(CXXFLAGS) -c -o $@ $<

./obj/%.o:	%.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

./obj/%_Dict.o:	./inc/%.h ./inc/%LinkDef.h
	@echo "Generating dictionary $@.."
	@echo ${ROOT_CntCln} -f $(basename $@).cpp  $?
	${ROOT_CntCln} -f $(basename $@).cpp  $?
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp
	cp -f -v $(basename $@)_rdict.pcm bin/
	cp -f -v $(basename $@)_rdict.pcm lib/

$(TARGET):	$(OBJECTS)
ifeq ($(PLATFORM),macosx)
	$(LD) $(SOFLAGS) $^ $(OutPutOpt) $@
	ln -sf $@ $(subst .$(DllSuf),.so,$@)
else
	$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(OutPutOpt) $@ $(EXPLLINKLIBS)
endif
	@echo "$@ done"

########################################################
# dictionaries (which don't follow the implicit rule)
########################################################
./obj/VFITS_Dict.o:
	@echo "A Generating dictionary $@.."
	@echo ${ROOT_CntCln} -f $(basename $@).cpp -I$(FITSSYS)/include inc/VFITS.h inc/VFITSLinkDef.h
	${ROOT_CntCln} -f $(basename $@).cpp  -I$(FITSSYS)/include inc/VFITS.h inc/VFITSLinkDef.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp
	cp -f -v $(basename $@)_rdict.pcm bin/
	cp -f -v $(basename $@)_rdict.pcm lib/

./obj/VDisplay_Dict.o:
	@echo "A Generating dictionary $@.."
	@echo ${ROOT_CntCln} -f $(basename $@).cpp -I./inc/ $(VBFCFLAGS) $(VBFFLAG) $(GSLCFLAGS) $(GSLFLAG) ./inc/VDisplay.h ./inc/VDisplayLinkDef.h
	${ROOT_CntCln} -f $(basename $@).cpp -I./inc/ $(VBFCFLAGS) $(VBFFLAG) $(GSLCFLAGS) $(GSLFLAG) ./inc/VDisplay.h ./inc/VDisplayLinkDef.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp
	cp -f -v $(basename $@)_rdict.pcm bin/
	cp -f -v $(basename $@)_rdict.pcm lib/

./obj/VLightCurve_Dict.o:
	@echo "Generating dictionary $@..."
	@echo ${ROOT_CntCln} -f $(basename $@).cpp -I./inc/ $(VBFCFLAGS) $(VBFFLAG) $(GSLCFLAGS) $(GSLFLAG) ./inc/VLightCurve.h ./inc/VLightCurveData.h ./inc/VLightCurveLinkDef.h
	${ROOT_CntCln} -f $(basename $@).cpp -I./inc/ $(VBFCFLAGS) $(VBFFLAG) $(GSLCFLAGS) $(GSLFLAG) ./inc/VLightCurve.h ./inc/VLightCurveData.h ./inc/VLightCurveLinkDef.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp
	cp -f -v $(basename $@)_rdict.pcm bin/
	cp -f -v $(basename $@)_rdict.pcm lib/

./obj/VZDCF_Dict.o:
	@echo "Generating dictionary $@..."
	@echo ${ROOT_CntCln} -f $(basename $@).cpp ./inc/VZDCF.h ./inc/VZDCFData.h ./inc/VZDCFLinkDef.h
	${ROOT_CntCln} -f $(basename $@).cpp ./inc/VZDCF.h ./inc/VZDCFData.h ./inc/VZDCFLinkDef.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp
	cp -f -v $(basename $@)_rdict.pcm bin/
	cp -f -v $(basename $@)_rdict.pcm lib/

###############################################################################################################################
# code which requires the libnova package installed in $LIBNOVASYS
#
# (note: experimental state)
###############################################################################################################################

./obj/binaryVisibility.o:	binaryVisibility.cpp
	$(CXX) $(CXXFLAGS) -I. -I  $(LIBNOVASYS)/include/ -c -o $@ $<

./obj/VLibNovaStar.o:	VLibNovaStar.cpp VLibNovaStar.h
	$(CXX) $(CXXFLAGS) -I. -I  $(LIBNOVASYS)/include/ -c -o $@ $<

./obj/VLibNovaSunAndMoon.o:	VLibNovaSunAndMoon.cpp VLibNovaSunAndMoon.h
	$(CXX) $(CXXFLAGS) -I. -I  $(LIBNOVASYS)/include/ -c -o $@ $<

binaryVisibility:	./obj/VLibNovaStar.o ./obj/VLibNovaSunAndMoon.o ./obj/binaryVisibility.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) -L$(LIBNOVASYS)/lib/ -lnova $(OutPutOpt) ./bin/$@
	@echo "$@ done"


########################################################
# writeFITS_eventlist
########################################################
writeFITS_eventlistOBJ	= ./obj/writeFITS_eventlist.o \
			  ./obj/CData.o \
			  ./obj/VSkyCoordinates.o \
			  ./obj/VSkyCoordinatesUtilities.o \
			  ./obj/VDB_Connection.o \
			  ./obj/VStarCatalogue.o  ./obj/VStarCatalogue_Dict.o \
			  ./obj/VStar.o ./obj/VStar_Dict.o \
			  ./obj/VUtilities.o  \
			  ./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			  ./obj/VAstronometry.o ./obj/VAstronometry_Dict.o

ifeq ($(ASTRONMETRY),-DASTROSLALIB)
    writeEventListTMVAOBJ += ./obj/VASlalib.o
endif

./obj/writeFITS_eventlist.o:	./src/writeFITS_eventlist.cpp
	$(CXX) $(CXXFLAGS) -I $(EVLIOSYS)/records/ -I $(EVLIOSYS)/include/ -c -o $@ $<

writeFITS_eventlist:	$(writeFITS_eventlistOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) -L $(EVLIOSYS)/lib -lfitsrecord $(OutPutOpt) ./bin/$@
	@echo "$@ done"

###############################################################################################################################
# print environment and compilation parameters
###############################################################################################################################
printconfig configuration config:
	@echo ""
	@echo "CONFIGURATION SUMMARY FOR $(package) version $(version)"
	@echo "======================================================"
	@echo ""
	@echo "$(CXX) $(GCCVERSION) on $(GCCMACHINE) $(ARCH)"
	@echo "    $(GCC_VER_MAJOR) $(GCC_VER_MINOR) $(GCC_GT_4_8)"
	@echo "    $(CXXFLAGS)"
	@echo "    $(GLIBS)"
	@echo ""
	@echo "using root version $(ROOTVERSION)"
	@echo "    $(ROOT_MYSQL)"
	@echo "    $(ROOTSYS)"
	@echo ""
ifeq ($(GSLFLAG),-DNOGSL)
	@echo "evndisp without GSL libraries (no Hough muon calibration, no likelihood fitter)"
else
	@echo "evndisp with GSL libraries (used in Hough muon calibration, likelihood fitter)"
	@echo "   GSL  $(GSLFLAG)" 
	@echo "   $(GSLCFLAGS) $(GSLLIBS)"
endif
	@echo ""
ifeq ($(VBFFLAG),-DNOVBF)
	@echo "evndisp without VBF support"
else
	@echo "evndisp with VBF support"
	@echo "    VBFSYS $(VBFSYS)"
	@echo "    VBFCFLAGS $(VBFCFLAGS)"
	@echo "    VBFLIBS $(VBFLIBS)"
endif
ifeq ($(VBFFLAG),-DVBF_034)
	@echo "    VBF support includes additional corsika info (version 0.34)"
endif
	@echo ""
ifeq ($(DBFLAG),-DRUNWITHDB)
	@echo "evndisp with database (mysql) support"
else
	@echo "evndisp without database (mysql) support"
endif
	@echo ""
ifeq ($(FITS),FALSE)
	@echo "no FITS support enabled"
else
	@echo "FITS support enabled $(FITSSYS)"
endif
	@echo ""
ifeq ($(ASTRONMETRY),-DASTROSOFA)
	@echo "Astronometry with SOFALIB $(SOFASYS)"
else
	@echo "Astronometry with SLALIB"
endif

	@echo ""

	@echo "Testing EVNDISP environmental variables (for VTS):"
	@echo "----------------------------------------------------------"
ifeq ($(strip $(EVNDISPSYS)),)
	@echo "EVNDISPSYS not set (see README/INSTALL)"
else
	@echo "EVNDISPSYS set to $(EVNDISPSYS)"
endif
ifeq ($(strip $(VERITAS_EVNDISP_AUX_DIR)),)
	@echo "VERITAS_EVNDISP_AUX_DIR not set (see README/INSTALL)"
else
	@echo "VERITAS_EVNDISP_AUX_DIR set to $(VERITAS_EVNDISP_AUX_DIR)"
endif
ifeq ($(strip $(VERITAS_DATA_DIR)),)
	@echo "VERITAS_DATA_DIR not set (see README/INSTALL)"
else
	@echo "VERITAS_DATA_DIR set to $(VERITAS_DATA_DIR)"
endif
ifeq ($(strip $(VERITAS_USER_DATA_DIR)),)
	@echo "VERITAS_USER_DATA_DIR not set (see README/INSTALL)"
else
	@echo "VERITAS_USER_DATA_DIR set to $(VERITAS_USER_DATA_DIR)"
endif
ifeq ($(strip $(VERITAS_IRFPRODUCTION_DIR)),)
	@echo "VERITAS_IRFPRODUCTION_DIR not set (see README/INSTALL)"
else
	@echo "VERITAS_IRFPRODUCTION_DIR set to $(VERITAS_IRFPRODUCTION_DIR)"
endif

###############################################################################################################################
# source code formating
###############################################################################################################################
formatSourceCode:
	@echo ""
	astyle --options=./.astylerc src/*
	astyle --options=./.astylerc inc/*
	astyle --options=./.astylerc macros/*.C macros/VTS/*.C

###############################################################################################################################
install:	all
	@echo "EVNDISP install: see ./bin/ and ./lib/ directories"

###############################################################################################################################
clean:
	-rm -f ./obj/*.o ./obj/*_Dict.cpp ./obj/*_Dict.h ./lib/*.pcm ./obj/*.pcm ./bin/*.pcm

rclean:
	-rm -f ./obj/*.o ./obj/*_Dict.cpp ./obj/*_Dict.h ./bin/* ./lib/libVAnaSum.so ./lib/*.pcm ./obj/*dict.pcm ./bin/*.pcm
###############################################################################################################################

.PHONY: all clean install TESTFITS configuration
