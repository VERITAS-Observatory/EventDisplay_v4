//! VTMVAEvaluator use a TMVA weight file for energy dependent gamma/hadron separation

#ifndef  VTMVAEvaluator_H
#define  VTMVAEvaluator_H

#include "CData.h"
#include "VGlobalRunParameter.h"
#include "VMathsandFunctions.h"
#include "VHistogramUtilities.h"
#include "VPlotUtilities.h"
#include "VStatistics.h"
#include "VTMVARunData.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TGraphAsymmErrors.h"
#include "TGraphSmooth.h"
#include "TLine.h"
#include "TMath.h"
#include "TMVA/Config.h"
#include "TMVA/Configurable.h"
#include "TMVA/Factory.h"
#include "TMVA/MethodCuts.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

using namespace std;

// data class for VTMVAEvaluator
// (per bin)
class VTMVAEvaluatorData : public TNamed
{
	public:
	
		unsigned int      fCounter;                       // needed???
		string            fTMVAFileName;
		string            fTMVAFileNameXML;
		string            fTMVAMethodTag;
		string            fTMVAMethodTag_2;
		
		double            fEnergyCut_Log10TeV_min;
		double            fEnergyCut_Log10TeV_max;
		double            fSpectralWeightedMeanEnergy_Log10TeV;
		double            fZenithCut_min;
		double            fZenithCut_max;
		
		double            fSignalEfficiency;
		double            fBackgroundEfficiency;
		double            fTMVACutValue;
		bool              fTMVAOptimumCutValueFound;
		double            fSourceStrengthAtOptimum_CU;
		double            fAngularContainmentRadius;
		double            fAngularContainmentFraction;
		
		TMVA::Reader*     fTMVAReader;                       //!
		
		VTMVAEvaluatorData();
		~VTMVAEvaluatorData() {}
		void print();
		
		ClassDef( VTMVAEvaluatorData, 1 );
};

///////////////////////////////////////////////////////////////////////////////

// data class for VTMVAEvaluator
// (this data is propagated into VGammaHadronCuts; used in VSensitivityCalculator)
class VTMVAEvaluatorResults : public TNamed
{
	public:
	
		vector< VTMVAEvaluatorData* > fTMVAData;
		
		VTMVAEvaluatorResults() {}
		~VTMVAEvaluatorResults() {}
		
		ClassDef( VTMVAEvaluatorResults, 8 );
};

///////////////////////////////////////////////////////////////////////////////

class VTMVAEvaluator : public TNamed, public VPlotUtilities
{
	private:
	
		bool     fDebug;
		bool     fIsZombie;
		
		VTMVAEvaluatorResults*  fTMVAEvaluatorResults;
		vector< VTMVAEvaluatorData* > fTMVAData;
		
		CData*   fData;
		
		map< unsigned int, double > fSignalEfficiencyMap;         // from user: energy dependent signal efficiency
		double                      fSignalEfficiencyNoVec;
		map< unsigned int, double > fTMVACutValueMap;
		double                      fTMVACutValueNoVec;
		
		string                  fParticleNumberFileName;          // particle numbers are read from this file
		double                  fParticleNumberFile_Conversion_Rate_to_seconds;
		double                  fOptimizationSourceSignificance;
		double                  fOptimizationFixedSignalEfficiency;
		double                  fOptimizationMinSourceStrength;
		double                  fOptimizationMinSignalEvents;
		double                  fOptimizationMinBackGroundEvents;
		double                  fOptimizationBackgroundAlpha;
		double                  fOptimizationObservingTime_h;
		double                  fTMVAAngularContainmentThetaFixedMinRadius;
		
		bool     fTMVAIgnoreTheta2Cut;           // ignore theta2 cut in TMVA
		bool     fTMVAThetaCutVariableSet;       // check if TMVA provides a theta2 cut variable
		double   fTMVA_EvaluationResult;         // result from TVMA evaluator
		bool     fTMVA_OptimizeAngularContainment; // optimize angular containment (using angular resolution vs containment histograms
		
		bool     fSmoothAndInterpolateMVAValues;
		
		string   fTMVAMethodName;
		double   fTMVAngularContainmentRadiusMax;     // maximum angular containment radius (optimization scales relative to this value)
		double   fTMVAErrorFraction_min;             // remove bins from background efficiency curves with large errors
		
		double   fSpectralIndexForEnergyWeighting;        // used to calculate the spectral weighted mean of an energy bin
		
		// gamma/hadron separation variables
		float    fNImages;
		float    fMSCW;
		float    fMSCL;
		float    fMWR;
		float    fMLR;
		float    fEmissionHeight;
		float    fEmissionHeightChi2_log10;
		unsigned int fEnergyReconstructionMethod;
		float    fEChi2S;
		float    fEChi2S_log10;
		float    fdES;
		float    fSizeSecondMax_log10;
		float    fTheta2;
		float    fCoreDist;
		float    fImages_Ttype[VDST_MAXTELESCOPES];
		float    fDispDiff;
		float    fDummy;
		
		bool     bPlotEfficiencyPlotsPerBin;
		
		TH1F*            getEfficiencyHistogram( string iName, TFile* iF, string iMethodTag_2 );
		double           getMeanEnergyAfterCut( TFile* f, double iCut, unsigned int iDataBin );
		bool             optimizeSensitivity( unsigned int iDataBin );
		TGraph*          fillfromGraph2D( TObject* i_G, double i_ze_min, double i_ze_max );
		void             fillTMVAEvaluatorResults();
		unsigned int     getDataBin();
		unsigned int     getDataBin( double iErec_log10TeV, double iZe );
		double           getSignalEfficiency( unsigned int iEbin, double iE_min, double iE_max, unsigned int iZbin, double iZ_min, double iZ_max );
		double           getTMVACutValue( unsigned int iEnergyBin, double iE_min_log10, double iE_max_log10, unsigned int iZenithBin, double iZ_min, double iZ_max );
		bool             getValuesFromEfficiencyHistograms( unsigned int iB );
		double           getValueFromMap( map< unsigned int, double > iDataMap, double iDefaultData, unsigned int iEnergyBin, double iE_min_log10, double iE_max_log10,
										  unsigned int iZenithBin, double iZ_min, double iZ_max, string iVariable );
		vector< string > getTrainingVariables( string iFile, vector< bool >& iSpectator );
		void             getOptimalAngularContainmentRadius( double effS, double effB, double Ndif, double Nof,
				TH2D* iHAngContainment, double iEnergy_log10_TeV,
				double& i_Signal_to_sqrtNoise, double& i_AngularContainmentRadius,
				double& i_AngularContainmentFraction );
		void             plotEfficiencyPlotsPerBin( unsigned int iBin,
				TGraph* iGSignal_to_sqrtNoise, TGraph* iGSignal_to_sqrtNoise_Smooth,
				TH1F* hEffS, TH1F* hEffB,
				TGraph* iGSignalEvents, TGraph* iGBackgroundEvents,
				TGraph* iGOpt_AngularContainmentRadius, TGraph* iGOpt_AngularContainmentFraction );
		TGraph*          readNonNoffGraphsFromFile( TFile* iF, double i_ze_min, double i_ze_max, bool bIsOn = true );
		void             reset();
		void             smoothAndInterPolateMVAValue( TH1F*, TH1F*, unsigned int iE_min, unsigned int iE_max, unsigned int iZ_min, unsigned int iZ_max );
		
	public:
	
		VTMVAEvaluator();
		~VTMVAEvaluator() {};
		
		bool    evaluate();
		TGraph* getOptimalTheta2Cut_Graph();
		vector< double > getBackgroundEfficiency();
		vector< bool >   getOptimumCutValueFound();
		vector< double > getSignalEfficiency();
		double  getOptimalTheta2Cut( double iEnergy_log10TeV, double iZe = -9999 );
		vector< double > getTMVACutValue();
		VTMVAEvaluatorResults* getTMVAEvaluatorResults()
		{
			return fTMVAEvaluatorResults;
		}
		bool   getTMVAThetaCutVariable()
		{
			return fTMVAThetaCutVariableSet;
		}
		double getTMVA_EvaluationResult()
		{
			return fTMVA_EvaluationResult;
		}
		bool   initializeWeightFiles( string iWeightFileName, unsigned int iWeightFileIndex_Emin, unsigned int iWeightFileIndex_Emax,
									  unsigned int iWeightFileIndex_Zmin, unsigned int iWeightFileIndex_Zmax, double iEnergyStepSize = 0.2, string iInstrumentEpoch = "noepoch" );
		bool   initializeDataStrutures( CData* iC );
		bool   IsZombie()
		{
			return fIsZombie;
		}
		TGraphAsymmErrors* plotSignalAndBackgroundEfficiencies( bool iLogY = true, double iYmin = 1.e-4, double iMVA_min = -1., double iMVA_max = 1. );
		void   printAngularContainmentRadius();
		void   printSensitivityOptimizationParameters();
		void   printSignalEfficiency();
		void   printSourceStrength_CU();
		void   setDebug( bool iB = false )
		{
			fDebug = iB;
		}
		void   setIgnoreTheta2Cut( bool iB = false )
		{
			fTMVAIgnoreTheta2Cut = iB;
		}
		void   setSensitivityOptimizationParameters( double iSignificance = 5., double iMinEvents = 10., double iObservationTime_h = 50.,
				double iMinBackgroundRateRatio = 1. / 5, double iMinBackgroundEvents = 0. )
		{
			fOptimizationSourceSignificance = iSignificance;
			fOptimizationMinSignalEvents = iMinEvents;
			fOptimizationMinBackGroundEvents = iMinBackgroundEvents;
			fOptimizationBackgroundAlpha = iMinBackgroundRateRatio;
			fOptimizationObservingTime_h = iObservationTime_h;
		}
		
		void   setSensitivityOptimizationFixedSignalEfficiency( double iOptimizationFixedSignalEfficiency = 1. );
		void   setSensitivityOptimizationMinSourceStrength( double iOptimizationMinSourceStrength = 0.001 );
		void   setOptimizeAngularContainment( bool iO = true )
		{
			fTMVA_OptimizeAngularContainment = iO;
		}
		void   setParticleNumberFile( string iParticleNumberFile = "", double iConversionFactor_to_seconds = 60. )
		{
			fParticleNumberFileName = iParticleNumberFile;
			fParticleNumberFile_Conversion_Rate_to_seconds = iConversionFactor_to_seconds;
		}
		void   setPlotEfficiencyPlotsPerBin( bool iB = false )
		{
			bPlotEfficiencyPlotsPerBin = iB;
		}
		void   setSignalEfficiency( double iSignalEfficiency = -99. );
		void   setSignalEfficiency( map< unsigned int, double > iMSignalEfficiency );
		void   setSmoothAndInterPolateMVAValues( bool iS = true )
		{
			fSmoothAndInterpolateMVAValues = iS;
		}
		void   setSpectralIndexForEnergyWeighting( double iS = -2. )
		{
			fSpectralIndexForEnergyWeighting = iS;
		}
		void   setTMVAAngularContainmentThetaFixedMinRadius( double iR = 0. )
		{
			fTMVAAngularContainmentThetaFixedMinRadius = iR;
		}
		void   setTMVAAngularContainmentRadiusMax( double iC = 0.68 )
		{
			fTMVAngularContainmentRadiusMax = iC;
		}
		void   setTMVACutValue( double iE = -99. );
		void   setTMVACutValue( map< unsigned int, double > iMVA );
		void   setTMVAErrorFraction( double iTMVAErrorFraction_min = 0.2 )
		{
			fTMVAErrorFraction_min = iTMVAErrorFraction_min;
		}
		void   setTMVAThetaCutVariable( bool iB = false )
		{
			fTMVAThetaCutVariableSet = iB;
		}
		void   setTMVAMethod( string iMethodName = "BDT" );
		
		ClassDef( VTMVAEvaluator, 31 );
};

#endif
