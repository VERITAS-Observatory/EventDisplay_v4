##########################################################################
# Makefile for eventdisplay analysis package
##########################################################################
#
#  for VERITAS: make VTS
#
#  for CTA:     make CTA
#
#  shell variables needed:
#    ROOTSYS (pointing to your root installation)
#
#  Libraries needed either for CTA or VTS:
#
#  for reading of VBF files (optional, VERITAS only)
#    VBFSYS  (pointing to VBF installation)
#    or
#   vbfConfig exists
#
#  for reading of sim_telarray (HESSIO) files (optional, CTA only)
#    HESSIOSYS (pointing to HESSIO installation)
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
# Gernot Maier 
##########################################################################
SHELL = /bin/sh
ARCH = $(shell uname)

#############################
# basic numbers 
#############################
package = EVNDISP
version = 4.40
distdir = $(package)-$(version)
ctapara = $(distdir).CTA.runparameter
vtspara = $(distdir).VTS.runparameter
#############################
# check compiler
GCCVERSION=$(shell gcc -dumpversion)
GCCMACHINE=$(shell gcc -dumpmachine)
#############################
#############################
# check root version number
#############################
ROOTVERSION=$(shell root-config --version)
ROOT528=$(shell expr 5.28 \>= `root-config --version | cut -f1 -d \/`)
#############################
# check for root libraries
#############################
ROOT_MLP=$(shell root-config --has-xml)
ROOT_MINUIT2=$(shell root-config --has-minuit2)
ROOT_MYSQL=$(shell root-config --has-mysql)
ROOT_DCACHE=$(shell root-config --has-dcache)
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
# DCACHE
# (necessary for CTA data analysis)
#############################
# check that root is compiled with dcache
DCTEST=$(shell root-config --has-dcache)
ifeq ($(DCTEST),yes)
  DCACHEFLAG=-DRUNWITHDCACHE
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
endif
#####################
# CTA HESSIO INPUT
#####################
# USE HESSIO LIBRARY
# (necessary for CTA hessio to VDST converter)
ifeq ($(strip $(HESSIOSYS)),)
HESSIO = FALSE
endif
#####################
# FITS ROUTINES
# (optional, necessary for root to FITS converter)
#####################
ifeq ($(strip $(FITSSYS)),)
FITS = FALSE
endif

########################################################################################################################
# compile and linker flags
########################################################################################################################
# compiler and linker general values
CXX           = g++
CXXFLAGS      = -O3 -g -Wall -fPIC -fno-strict-aliasing  -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_SOURCE -D_LARGEFILE64_SOURCE
CXXFLAGS     += -I. -I./inc/
CXXFLAGS     += $(VBFFLAG) $(DBFLAG) $(GSLFLAG) $(DCACHEFLAG) 
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
CXXFLAGS    += -Wdeprecated-declarations -stdlib=libc++
LDFLAGS       = -bind_at_load
DllSuf        = dylib
UNDEFOPT      = dynamic_lookup
SOFLAGS       = -dynamiclib -single_module -undefined $(UNDEFOPT)
endif

########################################################
# CXX FLAGS (taken from root)
########################################################
ROOTCFLAGS   = $(shell root-config --auxcflags)
CXXFLAGS     += $(ROOTCFLAGS)
CXXFLAGS     += -I$(shell root-config --incdir) -I$(shell root-config --incdir)/TMVA 
########################################################
# root libs
########################################################
ROOTGLIBS     = $(shell root-config --glibs)
GLIBS         = $(ROOTGLIBS)
GLIBS        += -lMLP -lTreePlayer -lTMVA -lMinuit -lXMLIO -lSpectrum
ifeq ($(ROOT_MINUIT2),yes)
   GLIBS     += -lMinuit2
endif

#ifeq ($(DCTEST),yes)
#   GLIBS     += -lDCache
#endif
########################################################
# VBF
########################################################
ifneq ($(VBFFLAG),-DNOVBF)
VBFCFLAGS     = -I$(VBFSYS)/include/VBF/
VBFLIBS       = $(shell vbfConfig --ldflags --libs)
CXXFLAGS     += $(VBFCFLAGS)
#GLIBS        += $(VBFLIBS)
endif
########################################################
# GSL FLAGS
########################################################
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
CXXFLAGS	+= -I $(FITSSYS)/include/
endif
########################################################
# HESSIO 
########################################################
ifneq ($(HESSIO),FALSE)
HESSIOINCLUDEFLAGS = -I $(HESSIOSYS)/include/
#CXXFLAGS        += $(HESSIOINCLUDEFLAGS) -DCTA_MAX
# 2010 PROD1 production
# CXXFLAGS        += $(HESSIOINCLUDEFLAGS) -DCTA -DCTA_ULTRA
# 2011 PROD1 production for Leeds
# CXXFLAGS        += $(HESSIOINCLUDEFLAGS) -DCTA_ULTRA
# 2011 PROD1 SC 
# CXXFLAGS        += $(HESSIOINCLUDEFLAGS) -DCTA_SC=2
# 2013 PROD2
CXXFLAGS        += $(HESSIOINCLUDEFLAGS) -DCTA -DCTA_PROD2 -DCTA_PROD2_TRGMASK
# SC MST FLAGS (needs 201312 hessio version)
# CXXFLAGS        += $(HESSIOINCLUDEFLAGS) -DCTA -DCTA_SC=3
endif
########################################################
# profiler (gperftools)
########################################################
#GLIBS        += -L/afs/ifh.de/group/cta/scratch/maierg/software/lib/lib/ -ltcmalloc
#CXXFLAGS     += -fno-omit-frame-pointer

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
	VTS.getRun_TimeElevAzim

CTA:	evndisp \
        CTA.convert_hessio_to_VDST \
        printRunParameter \
	mscw_energy \
	combineLookupTables \
	makeEffectiveArea \
	trainTMVAforGammaHadronSeparation \
	slib \
	writeCTAWPPhysSensitivityFiles \
	writeParticleRateFilesFromEffectiveAreas

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
		./obj/VPEReader.o \
		./obj/VPETree.o \
		./obj/VPETree_Dict.o \
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
		./obj/VImageAnalyzerData.o \
		./obj/VCalibrationData.o \
		./obj/VDST.o \
		./obj/VDSTTree.o \
		./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VPedestalCalculator.o \
		./obj/VDeadChannelFinder.o \
		./obj/VSpecialChannel.o \
		./obj/VDeadTime.o \
		./obj/VEvndispRunParameter.o  ./obj/VEvndispRunParameter_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VReadRunParameter.o \
		./obj/VEventLoop.o \
		./obj/VEvndispData.o \
		./obj/VDBRunInfo.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VUtilities.o \
		./obj/VASlalib.o \
		./obj/VPointing.o \
	 	./obj/VPointingDB.o \
		./obj/VSkyCoordinates.o \
		./obj/VArrayPointing.o \
		./obj/VTargets.o \
		./obj/VStarCatalogue.o  ./obj/VStarCatalogue_Dict.o \
		./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/VTrackingCorrections.o \
		./obj/CorrectionParameters.o \
		./obj/Angle.o \
		./obj/PointingMonitor.o \
		./obj/VSkyCoordinatesUtilities.o \
		./obj/VHoughTransform.o \
		./obj/VDB_PixelDataReader.o \
		./obj/VDisplay.o  

FROGSOBJECTS =	./obj/VFrogs.o \
                ./obj/frogs.o \
                ./obj/VFrogParameters.o

MODELOBJECTS =  ./obj/VMinimizer.o \
		./obj/VModel3DFn.o \
		./obj/VModel3DData.o \
		./obj/VModel3DParameters.o \
		./obj/VModelLnL.o \
		./obj/VModel3D.o \
		./obj/VEmissionHeightCalculator.o

EVNOBJECTS += $(MODELOBJECTS) 

ifneq ($(ARCH),Darwin)
EVNOBJECTS += ./obj/VDisplay_Dict.o
endif

# add frogs objects
ifneq ($(GSLFLAG),-DNOGSL)
   EVNOBJECTS += $(FROGSOBJECTS)
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
	@echo "LINKING evndisp without GSL libraries (frogs)"
else
	@echo "LINKING evndisp with GSL libraries (frogs)"
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
MSCOBJECTS=	./obj/Cshowerpars.o ./obj/Cmodel3Dpars.o ./obj/Ctpars.o \
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
                ./obj/VASlalib.o \
		./obj/VMedianCalculator.o \
                ./obj/VSkyCoordinatesUtilities.o \
                ./obj/VDB_Connection.o \
		./obj/mscw_energy.o
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
		./obj/VASlalib.o \
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
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		./obj/VRunList.o ./obj/VRunList_Dict.o \
		./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VAnaSumRunParameter.o ./obj/VAnaSumRunParameter_Dict.o \
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		 ./obj/VUtilities.o 




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
		./obj/VASlalib.o \
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
		./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o \
		./obj/VAnalysisUtilities.o ./obj/VAnalysisUtilities_Dict.o \
		./obj/VRunList.o ./obj/VRunList_Dict.o \
		./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
		./obj/VAnaSumRunParameter.o ./obj/VAnaSumRunParameter_Dict.o \
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VTargets.o \
		./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
		./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/VDB_Connection.o \
		./obj/VUtilities.o 

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
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VSkyCoordinatesUtilities.o ./obj/VUtilities.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/VASlalib.o \
		./obj/VEnergySpectrumfromLiterature.o ./obj/VEnergySpectrumfromLiterature_Dict.o \
		./obj/makeEffectiveArea.o

./obj/makeEffectiveArea.o:	./src/makeEffectiveArea.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

makeEffectiveArea:	$(EFFOBJECT) ./obj/VASlalib.o ./obj/makeEffectiveArea.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# anasum
########################################################
ANASUMOBJECTS =	./obj/VAnaSum.o ./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o ./obj/CData.o \
                ./obj/VStereoHistograms.o \
		./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
                ./obj/VStereoAnalysis.o ./obj/VMonoPlots.o \
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
		./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o ./obj/VTableLookupRunParameter.o \
		./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
		./obj/VTableLookupRunParameter_Dict.o ./obj/VTargets.o ./obj/VASlalib.o \
		./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
		./obj/VDB_Connection.o \
		./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VSkyCoordinatesUtilities.o ./obj/VUtilities.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/anasum.o

./obj/anasum.o:	./src/anasum.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

anasum:	$(ANASUMOBJECTS)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# hiloAnalysis
########################################################

HILOOBJECTS =	./obj/VHiLoTools.o \
		./obj/hiloAnalysis.o

./obj/hiloAnalysis.o: ./src/hiloAnalysis.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

hiloAnalysis:	$(HILOOBJECTS)
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
		./obj/VCTASensitivityRequirements.o ./obj/VCTASensitivityRequirements_Dict.o \
		./obj/VDifferentialFlux.o ./obj/VDifferentialFlux_Dict.o \
		./obj/VSpectralFitter.o ./obj/VSpectralFitter_Dict.o \
		./obj/VEnergyThreshold.o ./obj/VEnergyThreshold_Dict.o \
		./obj/VEnergySpectrum.o ./obj/VEnergySpectrum_Dict.o \
		./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
		./obj/VStar.o ./obj/VStar_Dict.o \
		./obj/VASlalib.o ./obj/VASlalib_Dict.o \
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
		./obj/VHiLoTools.o ./obj/VHiLoTools_Dict.o \
		./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
		./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
		./obj/VInstrumentResponseFunctionData.o ./obj/VInstrumentResponseFunctionData_Dict.o \
		./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
		./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
		./obj/VDBTools.o ./obj/VDBTools_Dict.o \
		./obj/VTMVARunData.o ./obj/VTMVARunData_Dict.o \
		./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
		./obj/VTMVAEvaluator.o ./obj/VTMVAEvaluator_Dict.o \
		./obj/VInstrumentResponseFunctionRunParameter.o ./obj/VInstrumentResponseFunctionRunParameter_Dict.o \
		./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
		./obj/VLightCurve.o ./obj/VLightCurve_Dict.o ./obj/VLightCurveData.o \
		./obj/VLightCurveUtilities.o ./obj/VLightCurveUtilities_Dict.o \
		./obj/VLombScargle.o ./obj/VLombScargle_Dict.o \
		./obj/VZDCF.o ./obj/VZDCF_Dict.o ./obj/VZDCFData.o \
		./obj/VUtilities.o \
		./obj/VPlotRadialAcceptance.o ./obj/VPlotRadialAcceptance_Dict.o \
		./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
		./obj/Cshowerpars.o ./obj/Cmodel3Dpars.o \
		./obj/Ctpars.o \
		./obj/VPlotEvndispReconstructionParameter.o ./obj/VPlotEvndispReconstructionParameter_Dict.o \
		./obj/VImageParameter.o  \
		./obj/VPlotWPPhysSensitivity.o ./obj/VPlotWPPhysSensitivity_Dict.o \
		./obj/VPlotPPUT.o ./obj/VPlotPPUT_Dict.o \
		./obj/VSiteData.o \
		./obj/VPlotTMVAParameters.o ./obj/VPlotTMVAParameters_Dict.o \
		./obj/VWPPhysSensitivityPlotsMaker.o ./obj/VWPPhysSensitivityPlotsMaker_Dict.o \
		./obj/VPedestalLowGain.o ./obj/VPedestalLowGain_Dict.o \
		./obj/VCTARequirements.o ./obj/VCTARequirements_Dict.o \
		./obj/VLowGainCalibrator.o ./obj/VLowGainCalibrator_Dict.o \
		./obj/VTimeMask.o ./obj/VTimeMask_Dict.o

ifeq ($(ROOT_MINUIT2),yes)
  SHAREDOBJS	+= ./obj/VSourceGeometryFitter.o ./obj/VSourceGeometryFitter_Dict.o
endif

ifneq ($(FITS),FALSE)
  SHAREDOBJS	+= ./obj/VFITS.o # ../obj/VFITS_Dict.o
endif

slib lsib ./lib/libVAnaSum.so:   $(SHAREDOBJS)
	mkdir -p ./lib
	$(LD) $(SOFLAGS) $(SHAREDOBJS) $(GLIBS) $(OutPutOpt) ./lib/libVAnaSum.so
ifneq ($(ROOT_MINUIT2),yes)
	@echo "ROOT NOT COMPILED WITH MINUIT2"
	@echo "THEREFORE: NO SOURCE GEOMETRY FITTER AVAILABLE"
endif
ifeq ($(FITS),FALSE)
	@echo "NO FITS LIBRARIES FOUND, NO FITS SUPPORT"
else
	@echo "SHARED LIBRARIES WITH FITS SUPPORT"
endif
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
                ./obj/VASlalib.o \
                ./obj/VSkyCoordinatesUtilities.o \
                ./obj/VDB_Connection.o \
		./obj/printRunParameter.o

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
                        ./obj/VASlalib.o \
                        ./obj/VSkyCoordinatesUtilities.o \
                        ./obj/VDB_Connection.o \
			./obj/VUtilities.o \
			./obj/makeDISPTables.o

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
                        ./obj/VASlalib.o \
                        ./obj/VSkyCoordinatesUtilities.o \
                        ./obj/VDB_Connection.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VEvndispReconstructionParameter.o ./obj/VEvndispReconstructionParameter_Dict.o \
			./obj/VDispTable.o ./obj/VDispTableReader.o ./obj/VDispTableReader_Dict.o \
			./obj/Cshowerpars.o ./obj/Ctpars.o \
			./obj/printDISPTables.o

./obj/printDISPTables.o:	./src/printDISPTables.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

printDISPTables:	$(PRINTDISPTABLESOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# compareDatawithMC
########################################################
COMPAREDATAMCOBJ=	./obj/VTargets.o \
                        ./obj/VASlalib.o \
			./obj/CData.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
			./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
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
			./obj/VRunList.o ./obj/VRunList_Dict.o ./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
			./obj/VPlotUtilities.o ./obj/VPlotUtilities_Dict.o ./obj/Ctelconfig.o \
			./obj/VHistogramUtilities.o ./obj/VHistogramUtilities_Dict.o \
			./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
			./obj/compareDatawithMC.o

./obj/compareDatawithMC.o:	./src/compareDatawithMC.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

compareDatawithMC:	$(COMPAREDATAMCOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# printBinaryOrbitalPhase
########################################################
PRINTBINARYOBJ=		./obj/VASlalib.o ./obj/printBinaryOrbitalPhase.o

./obj/printBinaryOrbitalPhase.o:	./src/printBinaryOrbitalPhase.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

printBinaryOrbitalPhase:	$(PRINTBINARYOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"


########################################################
# writeCTAEventListFromAnasum
# for converting post-cuts event lists to CTA's format
########################################################

# remove ./obj/VTargets.o AS SOON AS POSSIBLE!
writeCTAEventListFromAnasumOBJ  = $(SHAREDOBJS)
writeCTAEventListFromAnasumOBJ += ./obj/Angle.o
writeCTAEventListFromAnasumOBJ += ./obj/CorrectionParameters.o
writeCTAEventListFromAnasumOBJ += ./obj/FITSRecord.o 
writeCTAEventListFromAnasumOBJ += ./obj/PointingMonitor.o
writeCTAEventListFromAnasumOBJ += ./obj/VDBRunInfo.o
writeCTAEventListFromAnasumOBJ += ./obj/VPointingDB.o
writeCTAEventListFromAnasumOBJ += ./obj/VTargets.o
writeCTAEventListFromAnasumOBJ += ./obj/VTimeMask.o
writeCTAEventListFromAnasumOBJ += ./obj/VTimeMask_Dict.o
writeCTAEventListFromAnasumOBJ += ./obj/VTrackingCorrections.o
writeCTAEventListFromAnasumOBJ += ./obj/writeCTAEventListFromAnasum.o

./obj/writeCTAEventListFromAnasum.o:   ./src/writeCTAEventListFromAnasum.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

writeCTAEventListFromAnasum:   $(writeCTAEventListFromAnasumOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# writeCTAWPPhysSensitivityFiles 
########################################################
WRITECTAPHYSOBJ=	./obj/VWPPhysSensitivityFile.o \
			./obj/writeCTAWPPhysSensitivityFiles.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
			./obj/VASlalib.o \
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
			./obj/VStatistics_Dict.o \
			./obj/VUtilities.o \
			./obj/Ctelconfig.o

./obj/writeCTAWPPhysSensitivityFiles.o: 	./src/writeCTAWPPhysSensitivityFiles.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

writeCTAWPPhysSensitivityFiles:	$(WRITECTAPHYSOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# writeVTSWPPhysSensitivityFiles 
########################################################
WRITEVTSPHYSOBJ=	./obj/VWPPhysSensitivityFile.o \
			./obj/writeVTSWPPhysSensitivityFiles.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/CRunSummary.o ./obj/CRunSummary_Dict.o \
			./obj/VASlalib.o \
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

./obj/writeVTSWPPhysSensitivityFiles.o: 	./src/writeVTSWPPhysSensitivityFiles.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

writeVTSWPPhysSensitivityFiles:	$(WRITEVTSPHYSOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# writeParticleRateFilesFromEffectiveAreas 
########################################################
WRITECTAPHYSOBJ=	./obj/writeParticleRateFilesFromEffectiveAreas.o \
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

./obj/writeParticleRateFilesFromEffectiveAreas.o: 	./src/writeParticleRateFilesFromEffectiveAreas.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

writeParticleRateFilesFromEffectiveAreas:	$(WRITECTAPHYSOBJ)
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
					./obj/VASlalib.o \
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
UPDATEDBLASERRUN=	./obj/VDBTools.o ./obj/VDBTools_Dict.o \
			./obj/VStar.o ./obj/VStar_Dict.o \
			./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
			./obj/VExposure.o ./obj/VExposure_Dict.o \
			./obj/VDB_Connection.o \
			./obj/VASlalib.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VUtilities.o \
			./obj/VDB_CalibrationInfo.o \
			./obj/updateDBlaserRUN.o

./obj/updateDBlaserRUN.o: ./src/updateDBlaserRUN.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#updateDBlaserRUN: ./obj/updateDBlaserRUN.o
updateDBlaserRUN: $(UPDATEDBLASERRUN)
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "Done updateDBlaserRUN"


########################################################
# combineEffectiveAreas
########################################################
./obj/combineEffectiveAreas.o:	./src/combineEffectiveAreas.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

combineEffectiveAreas:	 ./obj/combineEffectiveAreas.o  \
			 ./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			 ./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			 ./obj/VGammaHadronCutsStatistics.o ./obj/VGammaHadronCutsStatistics_Dict.o \
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
			 ./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			 ./obj/VSkyCoordinatesUtilities.o ./obj/VUtilities.o \
			 ./obj/VMathsandFunctions.o ./obj/VMathsandFunctions_Dict.o \
			 ./obj/VASlalib.o \
			 ./obj/VGammaHadronCuts.o ./obj/VGammaHadronCuts_Dict.o 


	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"


########################################################
# trainTMVAforGammaHadronSeparation
########################################################
MAKEOPTCUTTMVAOBJ=	./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
			./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
			./obj/VEffectiveAreaCalculatorMCHistograms.o ./obj/VEffectiveAreaCalculatorMCHistograms_Dict.o \
			./obj/VSpectralWeight.o ./obj/VSpectralWeight_Dict.o \
			./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
			./obj/VTableLookupRunParameter.o ./obj/VTableLookupRunParameter_Dict.o \
			./obj/VTMVARunData.o ./obj/VTMVARunData_Dict.o \
			./obj/VTMVARunDataEnergyCut.o ./obj/VTMVARunDataEnergyCut_Dict.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VUtilities.o \
			./obj/trainTMVAforGammaHadronSeparation.o

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
				./obj/trainTMVAforGammaHadronSeparation_TrainingFile.o

./obj/trainTMVAforGammaHadronSeparation_TrainingFile.o:	./src/trainTMVAforGammaHadronSeparation_TrainingFile.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

trainTMVAforGammaHadronSeparation_TrainingFile:	$(MAKEOPTCUTTMVATRAININGOBJ) 	
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "Done"

########################################################
# makeOptimizeBoxCutsbyParameterSpaceSearch
########################################################
./obj/makeOptimizeBoxCutsbyParameterSpaceSearch.o:	./src/makeOptimizeBoxCutsbyParameterSpaceSearch.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

makeOptimizeBoxCutsbyParameterSpaceSearch:	./obj/CData.o \
						./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
						./obj/makeOptimizeBoxCutsbyParameterSpaceSearch.o 
	$(LD) $(LDFLAGS) $^ $(GLIBS) -L./lib $(OutPutOpt) ./bin/$@
	@echo "$@ done"

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
                                ./obj/VASlalib.o \
                                ./obj/VSkyCoordinatesUtilities.o \
                                ./obj/VDB_Connection.o \
				./obj/VTS.calculateCrabRateFromMC.o
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
				./obj/VASlalib.o \
                                ./obj/VStar.o ./obj/VStar_Dict.o \
                                ./obj/VUtilities.o \
                                ./obj/VSkyCoordinatesUtilities.o \
				./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
				./obj/VUtilities.o \
				./obj/VTS.calculateExposureFromDB.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"	

########################################################
# VTS.getLaserRunFromDB
########################################################
./obj/VTS.getLaserRunFromDB.o:   ./src/VTS.getLaserRunFromDB.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTS.getLaserRunFromDB:	./obj/VDBTools.o ./obj/VDBTools_Dict.o \
			./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
			./obj/VStar.o ./obj/VStar_Dict.o \
                        ./obj/VUtilities.o \
                        ./obj/VASlalib.o \
                        ./obj/VSkyCoordinatesUtilities.o \
			./obj/VDBRunInfo.o \
			./obj/VDB_Connection.o \
			./obj/VASlalib.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VTS.getLaserRunFromDB.o

	$(LD) $(LDFLAGS) $^ $(GLIBS) $(OutPutOpt) ./bin/$@
	@echo "$@ done"

########################################################
# VTS.getRunListFromDB
########################################################
./obj/VTS.getRunListFromDB.o:   ./src/VTS.getRunListFromDB.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

VTS.getRunListFromDB:	./obj/VDBTools.o ./obj/VDBTools_Dict.o \
			./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
			./obj/VStar.o ./obj/VStar_Dict.o \
			./obj/VExposure.o ./obj/VExposure_Dict.o \
			./obj/VDB_Connection.o \
			./obj/VASlalib.o \
                        ./obj/VStarCatalogue.o ./obj/VStarCatalogue_Dict.o \
                        ./obj/VStar.o ./obj/VStar_Dict.o \
                        ./obj/VUtilities.o \
                        ./obj/VSkyCoordinatesUtilities.o \
			./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
			./obj/VUtilities.o \
			./obj/VTS.getRunListFromDB.o

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

VTS.next_day:	./obj/VFITS.o \
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
# HESSIO converter
########################################################

./obj/CTA.convert_hessio_to_VDST.o:	./src/CTA.convert_hessio_to_VDST.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

CTA.convert_hessio_to_VDST:	./obj/VDSTTree.o \
				./obj/VMonteCarloRunHeader.o ./obj/VMonteCarloRunHeader_Dict.o \
				./obj/VASlalib.o \
				./obj/VSkyCoordinatesUtilities.o \
				./obj/VEvndispRunParameter.o ./obj/VEvndispRunParameter_Dict.o \
				./obj/VImageCleaningRunParameter.o ./obj/VImageCleaningRunParameter_Dict.o \
				./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o \
				$(HESSIOSYS)/out/io_trgmask.o \
				./obj/CTA.convert_hessio_to_VDST.o
	$(LD) $(LDFLAGS) $^ $(GLIBS) -L$(HESSIOSYS)/lib -lhessio \
	$(OutPutOpt) ./bin/$@
	@echo "$@ done"

TESTHESSIO:
ifeq ($(HESSIO),FALSE)
	   @echo ""
	   @echo "----------------------------------------"
	   @echo "NO HESSIOSYS ENVIRONMENTAL VARIABLE SET"
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
	@echo rootcint -f $(basename $@).cpp -c -p $?
	@rootcint -f $(basename $@).cpp -c -p $?
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp

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

./obj/VDisplay_Dict.o:	
	@echo "A Generating dictionary $@.."
	@echo rootcint -f $(basename $@).cpp  -c -p -I./inc/ $(VBFCFLAGS) $(VBFFLAG) $(GSLCFLAGS) $(GSLFLAG)  ./inc/VDisplay.h ./inc/VDisplayLinkDef.h
	@rootcint -f $(basename $@).cpp  -c -p -I./inc/ $(VBFCFLAGS) $(VBFFLAG) $(GSLCFLAGS) $(GSLFLAG) ./inc/VDisplay.h ./inc/VDisplayLinkDef.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp

./obj/VLightCurve_Dict.o:	
	@echo "Generating dictionary $@..."
	@echo rootcint -f $(basename $@).cpp -c -p ./inc/VLightCurve.h ./inc/VLightCurveData.h ./inc/VLightCurveLinkDef.h
	@rootcint -f $(basename $@).cpp -c -p ./inc/VLightCurve.h ./inc/VLightCurveData.h ./inc/VLightCurveLinkDef.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp

./obj/VZDCF_Dict.o:	
	@echo "Generating dictionary $@..."
	@echo rootcint -f $(basename $@).cpp -c -p ./inc/VZDCF.h ./inc/VZDCFData.h ./inc/VZDCFLinkDef.h
	@rootcint -f $(basename $@).cpp -c -p ./inc/VZDCF.h ./inc/VZDCFData.h ./inc/VZDCFLinkDef.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(basename $@).cpp

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
			  ./obj/VASlalib.o \
			  ./obj/VTargets.o \
			  ./obj/VUtilities.o  \
			  ./obj/VGlobalRunParameter.o ./obj/VGlobalRunParameter_Dict.o


./obj/writeFITS_eventlist.o:	./src/writeFITS_eventlist.cpp
	$(CXX) $(CXXFLAGS) -I $(EVLIOSYS)/records/ -I $(EVLIOSYS)/include/ -c -o $@ $<

writeFITS_eventlist:	$(writeFITS_eventlistOBJ)
	$(LD) $(LDFLAGS) $^ $(GLIBS) -L $(EVLIOSYS)/lib -lfitsrecord $(OutPutOpt) ./bin/$@
	@echo "$@ done"


###############################################################################################################################
# make a tar package with all the source files / Makefiles / scripts
###############################################################################################################################
dist: $(distdir).tar.gz

$(distdir).tar.gz:	$(distdir)
	tar chof - $(distdir) | gzip -9 -c > $@
	rm -rf $(distdir)

$(distdir):	FORCEDISTDIR
	mkdir -p $(distdir)
	cp Makefile $(distdir)
	cp setObservatory.sh $(distdir)
	cp setObservatory.tcsh $(distdir)
	mkdir -p $(distdir)/obj
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/lib
	mkdir -p $(distdir)/README
	cp README/README* $(distdir)/README
	cp README/INSTALL $(distdir)/README
	cp README/AUTHORS $(distdir)/README
	cp README/CHANGELOG $(distdir)/README
	mkdir -p $(distdir)/doc
	cp doc/Manual.tex $(distdir)/doc
	cp doc/Manual_Title.tex $(distdir)/doc
	cp doc/Manual.pdf $(distdir)/doc
	mkdir -p $(distdir)/src
	cp src/*.cpp src/*.C src/*.c $(distdir)/src
	mkdir -p $(distdir)/inc
	cp inc/*.h $(distdir)/inc
	mkdir -p $(distdir)/macros
	cp -r macros/*.C $(distdir)/macros
	mkdir -p $(distdir)/macros/CTA
	cp -r macros/CTA/*.C $(distdir)/macros/CTA
	mkdir -p $(distdir)/macros/VTS
	cp -r macros/VTS/*.C $(distdir)/macros/VTS
	mkdir -p $(distdir)/scripts/VTS
	mkdir -p $(distdir)/scripts/CTA
	mkdir -p $(distdir)/scripts/CTA/grid-tools
	cp -r scripts/CTA/*.sh $(distdir)/scripts/CTA
	cp -r scripts/CTA/*.list $(distdir)/scripts/CTA
	cp -r scripts/CTA/CTA.EVNDISP* $(distdir)/scripts/CTA
	cp -r scripts/CTA/grid-tools/* $(distdir)/scripts/CTA/grid-tools
	cp -r scripts/VTS/*.sh $(distdir)/scripts/VTS
	cp -r scripts/VTS/VTS.EVNDISP* $(distdir)/scripts/VTS

FORCEDISTDIR:
	rm -rf $(distdir).tar.gz  >/dev/null 2>&1
	rm -rf $(distdir) >/dev/null 2>&1

###############################################################################################################################
# make a tar package with all run parameter files
###############################################################################################################################

#########################################
# CTA

CTA.runfiles:	$(ctapara).tar.gz

$(ctapara).tar.gz:	$(ctapara)
	find $(ctapara) \( -type f -o -type d \) -print | cpio -o -H ustar > $(ctapara).tar
	gzip $(ctapara).tar
	rm -rf $(ctapara)

$(ctapara):
	rm -rf $(ctapara).tar.gz  >/dev/null 2>&1
	rm -rf $(distdir) >/dev/null 2>&1
	mkdir -p $(ctapara)
	mkdir -p $(ctapara)/AstroData
	cp -r $(CTA_EVNDISP_AUX_DIR)/AstroData/TeV_data $(ctapara)/AstroData
	mkdir -p $(ctapara)/DetectorGeometry
	cp -r $(CTA_EVNDISP_AUX_DIR)/DetectorGeometry/prod1 $(ctapara)/DetectorGeometry
	cp -r $(CTA_EVNDISP_AUX_DIR)/DetectorGeometry/CTA.prod2* $(ctapara)/DetectorGeometry
	mkdir -p $(ctapara)/GammaHadronCutFiles
	cp -Lr $(CTA_EVNDISP_AUX_DIR)/GammaHadronCutFiles/ANA* $(ctapara)/GammaHadronCutFiles
	mkdir -p $(ctapara)/ParameterFiles
	cp -Lr $(CTA_EVNDISP_AUX_DIR)/ParameterFiles/EFFECTIVEAREA.runparameter $(ctapara)/ParameterFiles
	cp -Lr $(CTA_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.global.runparameter $(ctapara)/ParameterFiles
	cp -Lr $(CTA_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.prod*.runparameter $(ctapara)/ParameterFiles
	cp -Lr $(CTA_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.reconstruction.runparameter $(ctapara)/ParameterFiles
	cp -Lr $(CTA_EVNDISP_AUX_DIR)/ParameterFiles/TMVA.BDT.runparameter $(ctapara)/ParameterFiles
	cp -Lr $(CTA_EVNDISP_AUX_DIR)/ParameterFiles/scriptsInput.prod*.runparameter $(ctapara)/ParameterFiles
	mkdir -p $(ctapara)/RadialAcceptances/
	mkdir -p $(ctapara)/Calibration/
	mkdir -p $(ctapara)/EffectiveAreas/
	mkdir -p $(ctapara)/Tables/

#########################################
# VTS
#

# VTS.runfiles:	$(vtscalirf).tar.gz $(vtspara).tar.gz
VTS.runfiles:	$(vtspara).tar.gz

$(vtspara).tar.gz:	
	rm -rf $(vtspara).tar.gz  >/dev/null 2>&1
	rm -rf $(distdir) >/dev/null 2>&1
	mkdir -p $(vtspara)
# astrodata	
	mkdir -p $(vtspara)/AstroData
	rsync -av --exclude=".*" $(VERITAS_EVNDISP_AUX_DIR)/AstroData/Catalogues $(vtspara)/AstroData
	rsync -av --exclude=".*"  $(VERITAS_EVNDISP_AUX_DIR)/AstroData/TeV_data $(vtspara)/AstroData
# calibration directories
	mkdir -p $(vtspara)/Calibration/Tel_1
	mkdir -p $(vtspara)/Calibration/Tel_2
	mkdir -p $(vtspara)/Calibration/Tel_3
	cp -f $(VERITAS_EVNDISP_AUX_DIR)/Calibration/calibrationlist.dat $(vtspara)/Calibration/
	cp -f $(VERITAS_EVNDISP_AUX_DIR)/Calibration/calibrationlist.LowGain.dat $(vtspara)/Calibration/
# detector geometry
	mkdir -p $(vtspara)/DetectorGeometry
	cp -f $(VERITAS_EVNDISP_AUX_DIR)/DetectorGeometry/EVN_V4_Autumn2007_20130110.txt $(vtspara)/DetectorGeometry
	cp -f $(VERITAS_EVNDISP_AUX_DIR)/DetectorGeometry/EVN_V6_Upgrade_20121127_v420.txt $(vtspara)/DetectorGeometry
	cp -f $(VERITAS_EVNDISP_AUX_DIR)/DetectorGeometry/EVN_V5_Oct2012_newArrayConfig_20121027_v420.txt $(vtspara)/DetectorGeometry
	cp -f $(VERITAS_EVNDISP_AUX_DIR)/DetectorGeometry/EVN_V4_Oct2012_oldArrayConfig_20130428_v420.txt $(vtspara)/DetectorGeometry
# NSB files
	mkdir -p $(vtspara)/NOISE
# gamma hadron files
	mkdir -p $(vtspara)/GammaHadronCutFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/GammaHadronCutFiles/ANASUM.GammaHadron.d20131031* $(vtspara)/GammaHadronCutFiles
# run parameter files
	mkdir -p $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/ANASUM.runparameter $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/ANASUM.timemask.dat $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/ANASUM.runlist $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/EFFECTIVEAREA.runparameter $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.global.runparameter $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.reconstruction.runparameter $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.reconstruction.SW18_noDoublePass.runparameter $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.reconstruction.LGCalibration.runparameter $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.specialchannels.dat $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/EVNDISP.validchannels.dat $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/TMVA.BoxCuts.runparameter $(vtspara)/ParameterFiles
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/ParameterFiles/COMPAREMC.runparameter $(vtspara)/ParameterFiles
# radial acceptances, effective areas, tables
	mkdir -p $(vtspara)/RadialAcceptances
	mkdir -p $(vtspara)/EffectiveAreas
	mkdir -p $(vtspara)/Tables
	cd $(vtspara) && tar -zcvf ../$(vtspara).tar.gz . && cd ..
	rm -rf $(vtspara)

######
# VTS calibration and IRF files

$(vtscalirf).tar.gz:
	@echo "bbb "
	rm -rf $(vtspara).CAL.tar.gz  >/dev/null 2>&1
	rm -rf $(distdir) >/dev/null 2>&1
	mkdir -p $(vtspara)
	mkdir -p $(vtspara)/Calibration/Tel_1
	mkdir -p $(vtspara)/Calibration/Tel_2
	mkdir -p $(vtspara)/Calibration/Tel_3
	mkdir -p $(vtspara)/Calibration/Tel_4
	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_1/36862.lpe* $(vtspara)/Calibration/Tel_1/
	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_2/36862.lpe* $(vtspara)/Calibration/Tel_2/
	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_3/36862.lpe* $(vtspara)/Calibration/Tel_3/
	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_4/36862.lpe* $(vtspara)/Calibration/Tel_4/
	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_1/[0-9][0-9][0-9][0-9][0-9][0-9][0-9].lpe* $(vtspara)/Calibration/Tel_1/
	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_2/[0-9][0-9][0-9][0-9][0-9][0-9][0-9].lpe* $(vtspara)/Calibration/Tel_2/
	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_3/[0-9][0-9][0-9][0-9][0-9][0-9][0-9].lpe* $(vtspara)/Calibration/Tel_3/
	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_4/[0-9][0-9][0-9][0-9][0-9][0-9][0-9].lpe* $(vtspara)/Calibration/Tel_4/
#	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_1/*.lmult.root  $(vtspara)/Calibration/Tel_1/
#	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_2/*.lmult.root  $(vtspara)/Calibration/Tel_2/
#	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_3/*.lmult.root  $(vtspara)/Calibration/Tel_3/
#	cp -r $(VERITAS_EVNDISP_AUX_DIR)/Calibration/Tel_4/*.lmult.root  $(vtspara)/Calibration/Tel_4/
# NSB files for simulations
	mkdir -p $(vtspara)/NOISE
	cp -L $(VERITAS_EVNDISP_AUX_DIR)/NOISE/*.grisu $(vtspara)/NOISE
# radial acceptances, effective areas, tables
#	mkdir -p $(vtspara)/RadialAcceptances
#	cp -Lr $(VERITAS_EVNDISP_AUX_DIR)/RadialAcceptances/radialAcceptance-d20131115-* $(vtspara)/RadialAcceptances/
#	mkdir -p $(vtspara)/EffectiveAreas
#	cp -L $(VERITAS_EVNDISP_AUX_DIR)/EffectiveAreas/effArea-d20131031* $(vtspara)/EffectiveAreas/
#	mkdir -p $(vtspara)/Tables
#	cp -L $(VERITAS_EVNDISP_AUX_DIR)/Tables/table_d20131115_GrI* $(vtspara)/Tables/
# make tar file
	cd $(vtspara) && tar -zcvf ../$(vtspara).CAL.tar.gz . && cd ..
	rm -rf $(vtspara)

###############################################################################################################################
# print environment and compilation parameters
###############################################################################################################################
configuration config:
	@echo ""
	@echo "CONFIGURATION TEST FOR $(package) version $(version)"
	@echo "======================================================"
	@echo ""
	@echo "gcc $(GCCVERSION) on $(GCCMACHINE) $(ARCH)"
	@echo ""
	@echo "using root version $(ROOTVERSION)"
	@echo "    compiled with MLP: $(ROOT_MLP), MINUIT2: $(ROOT_MINUIT2), MYSQL: $(ROOT_MYSQL), DCACHE: $(ROOT_DCACHE), MATHMORE: $(ROOT_MATHMORE)"
	@echo ""
ifeq ($(GSLFLAG),-DNOGSL)
	@echo "evndisp without GSL libraries (no frogs, no Hough muon calibration)"
else
	@echo "evndisp with GSL libraries (frogs, Hough muon calibration)"
endif
ifeq ($(VBFFLAG),-DNOVBF)
	@echo "evndisp without VBF support"
else
	@echo "evndisp with VBF support"
endif
ifeq ($(DBFLAG),-DRUNWITHDB)
	@echo "evndisp with database support"
else
	@echo "evndisp without database support"
endif
ifeq ($(HESSIO),FALSE)
	@echo "no HESSIO support enabled"
else
	@echo "HESSIO support enabled"
endif
ifeq ($(FITS),FALSE)
	@echo "no FITS support enabled"
else
	@echo "FITS support enabled"
endif

###############################################################################################################################
# source code formating
###############################################################################################################################
formatSourceCode:
	@echo ""
	astyle --options=./.astylerc src/*
	astyle --options=./.astylerc inc/*
	astyle --options=./.astylerc macros/*.C macros/VTS/*.C macros/CTA/*.C

###############################################################################################################################
install:	all
	@echo "EVNDISP install: see ./bin/ and ./lib/ directories"

###############################################################################################################################
clean:
	-rm -f ./obj/*.o ./obj/*_Dict.cpp ./obj/*_Dict.h 
###############################################################################################################################

.PHONY: all clean install FORCEDISTDIR dist TESTHESSIO TESTFITS configuration
