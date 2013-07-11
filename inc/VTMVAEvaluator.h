//! VTMVAEvaluator use a TMVA weight file for energy dependent gamma/hadron separation
// Revision $Id: VTMVAEvaluator.h,v 1.1.2.3 2011/04/07 15:27:22 gmaier Exp $

#ifndef  VTMVAEvaluator_H
#define  VTMVAEvaluator_H

#include "CData.h"
#include "VGlobalRunParameter.h"
#include "VMathsandFunctions.h"
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
#include "TString.h"

using namespace std;

// data class for VTMVAEvaluator
// (this data is propagated into VGammaHadronCuts)
class VTMVAEvaluatorResults : public TNamed
{
   public:

    vector< double >        fEnergyCut_Log10TeV_min;
    vector< double >        fEnergyCut_Log10TeV_max;
    vector< double >        fSignalEfficiency;                // from user or best signal/sqrt(noise)
    vector< double >        fBackgroundEfficiency;            // from best signal/sqrt(noise)
    vector< double >        fTMVACutValue;
    vector< bool >          fTMVAOptimumCutValueFound;
    vector< double >        fSourceStrengthAtOptimum_CU;

    VTMVAEvaluatorResults() {}
   ~VTMVAEvaluatorResults() {}

   ClassDef(VTMVAEvaluatorResults, 1 );
};

class VTMVAEvaluator : public TNamed, public VPlotUtilities
{
   private:

   bool     fDebug;
   bool     fIsZombie;

   VTMVAEvaluatorResults*  fTMVAEvaluatorResults;

   CData*   fData;
   vector< TMVA::Reader* > fTMVAReader;
   vector< double >        fEnergyCut_Log10TeV_min;
   vector< double >        fEnergyCut_Log10TeV_max;
   vector< unsigned int >  fEnergyReconstructionMethod;
   vector< TString >       fTMVAMethodTag;
   vector< double >        fSignalEfficiency;                // from user or best signal/sqrt(noise)
   map< unsigned int, double > fSignalEfficiencyMap;         // from user: energy dependent signal efficiency
   vector< double >        fBackgroundEfficiency;            // from best signal/sqrt(noise)
   double                  fSignalEfficiencyNoVec;
   vector< double >        fTMVACutValue;
   map< unsigned int, double > fTMVACutValueMap;
   double                  fTMVACutValueNoVec;
   vector< bool >          fTMVAOptimumCutValueFound;
   vector< double >        fSourceStrengthAtOptimum_CU;

   string                  fParticleNumberFileName;          // particle numbers are read from this file
   double                  fOptmizationSourceSignificance;
   double                  fOptmizationFixedSignalEfficiency;
   double                  fOptmizationMinSignalEvents;
   double                  fOptmizationMinBackGroundEvents;
   double                  fOptimizationBackgroundAlpha;
   double                  fOptimizationObservingTime_h;
   double                  fTMVAOptimizationStepsize;

   bool     fTMVAIgnoreTheta2Cut;           // ignore theta2 cut in TMVA
   bool     fTMVAThetaCutVariableSet;       // check if TMVA provides a theta2 cut variable
   double   fTMVA_EvaluationResult;         // result from TVMA evaluator

   string   fTMVAMethodName;
   bool     fTMVAMethodName_BOXCUTS;
   unsigned int fTMVAMethodCounter;

   double   fTMVAErrorFraction_min;             // remove bins from background efficiency curves with large errors

   double   fSpectralIndexForEnergyWeighting;        // used to calculate the spectral weighted mean of an energy bin

// cut values (energy dependent)
   vector< double > fBoxCutValue_theta2;

   vector< vector< Double_t > > fBoxCutValue_min;
   vector< vector< Double_t > > fBoxCutValue_max;
   vector< vector< string > >   fBoxCutValue_Name;

// gamma/hadron separation variables
   float    fNImages; 
   float    fMSCW;
   float    fMSCL;
   float    fMWR;
   float    fMLR;
   float    fEmissionHeight;
   float    fEmissionHeightChi2_log10;
   float    fEChi2;
   float    fEChi2_log10;
   float    fdE;
   float    fEChi2S;
   float    fEChi2S_log10;
   float    fdES;
   float    fSizeSecondMax_log10;
   float    fTheta2;
   float    fCoreDist;
   float    fImages_Ttype[VDST_MAXTELESCOPES];
   float    fDummy;

   bool     bPlotEfficiencyPlotsPerEnergy;

   TH1F*            getEfficiencyHistogram( string iName, TFile *iF );
   bool             optimizeSensitivity( unsigned int i, string iTMVARootFile );
   void             fillTMVAEvaluatorResults();
   double           getSignalEfficiency( unsigned int iEbin, double iE_min, double iE_max );
   double           getTMVACutValue( unsigned int iEnergyBin, double iE_min_log10, double iE_max_log10 );
   bool             getValuesFromEfficiencyHistograms( double& iMVACut, double& iSignalEfficiency, double& iBackgroundEfficiency,
                                                       unsigned int iBin, string iWeightFileName );
   double           getValueFromMap( map< unsigned int, double > iDataMap, double iDefaultData,
                                     unsigned int iEnergyBin,
				     double iE_min_log10, double iE_max_log10, string iVariable );
   vector< string > getTrainingVariables( string iFile, vector< bool >& iSpectator  );
   void             plotEfficiencyPlotsPerEnergy( unsigned int iBin, 
                                                  TGraph* iGSignal_to_sqrtNoise, TGraph* iGSignal_to_sqrtNoise_Smooth,
						  TH1F* hEffS, TH1F* hEffB, 
						  double iEnergy_Log10TeV_min, double iEnergy_Log10TeV_max,
						  TGraph* iGSignalEvents, TGraph* iGBackgroundEvents );
   void     reset();

   public:

   VTMVAEvaluator();
  ~VTMVAEvaluator() {};

   bool    evaluate();
   double  getBoxCut_Theta2( double iEnergy_log10TeV );
   TGraph* getBoxCut_Theta_Graph();
   TGraph* getBoxCut_Theta2_Graph();
   vector< double > getBoxCut_Theta2() { return fBoxCutValue_theta2; }
   vector< double > getBackgroundEfficiency() { return fBackgroundEfficiency; }
   vector< bool >   getOptimumCutValueFound() { return fTMVAOptimumCutValueFound; }
   vector< double > getSignalEfficiency() { return fSignalEfficiency; }
   unsigned int     getSpectralWeightedEnergyBin();
   vector< double > getTMVACutValue() { return fTMVACutValue; }
   VTMVAEvaluatorResults* getTMVAEvaluatorResults() { return fTMVAEvaluatorResults; }
   bool   getTMVAThetaCutVariable() { return fTMVAThetaCutVariableSet; }
   double getTMVA_EvaluationResult() { return fTMVA_EvaluationResult; }
   bool   initializeWeightFiles( string iWeightFileName, unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max );
   bool   initializeDataStrutures( CData* iC );
   bool   isBoxCuts() { return fTMVAMethodName_BOXCUTS; }
   bool   IsZombie() { return fIsZombie; }
   void   plotBoxCuts();
   void   plotSignalAndBackgroundEfficiencies( bool iLogY = true, double iYmin = 1.e-4, double iMVA_min = -1., double iMVA_max = 1. );
   void   printSensitivityOptimizationParameters();
   void   printSignalEfficiency();
   void   printSourceStrength_CU();
   void   setDebug( bool iB = false ) { fDebug = iB; }
   void   setIgnoreTheta2Cut( bool iB = false ) { fTMVAIgnoreTheta2Cut = iB; }
   void   setSensitivityOptimizationParameters( double iSignificance = 5., double iMinEvents = 10., double iObservationTime_h = 50.,
				                double iMinBackgroundRateRatio = 1./5, double iMinBackgroundEvents = 0. )
          { fOptmizationSourceSignificance = iSignificance;
	    fOptmizationMinSignalEvents = iMinEvents;
	    fOptmizationMinBackGroundEvents = iMinBackgroundEvents; 
	    fOptimizationBackgroundAlpha = iMinBackgroundRateRatio;
	    fOptimizationObservingTime_h = iObservationTime_h; }

   void   setSensitivityOptimizationFixedSignalEfficiency( double iOptmizationFixedSignalEfficiency = 1. );
   void   setParticleNumberFile( string iParticleNumberFile = "" ) { fParticleNumberFileName = iParticleNumberFile; }
   void   setPlotEfficiencyPlotsPerEnergy( bool iB = false ) { bPlotEfficiencyPlotsPerEnergy = iB; }
   void   setSignalEfficiency( double iSignalEfficiency = -99. );
   void   setSignalEfficiency( map< unsigned int, double > iMSignalEfficiency );
   void   setSpectralIndexForEnergyWeighting( double iS = -2. )  { fSpectralIndexForEnergyWeighting = iS; }
   void   setTMVAOptimizationEnergyStepSize( double iStep = 0.25 ) { fTMVAOptimizationStepsize = iStep; }
   void   setTMVACutValue( double iE = -99. );
   void   setTMVACutValue( map< unsigned int, double > iMVA );
   void   setTMVAErrorFraction( double iTMVAErrorFraction_min = 0.2 ) { fTMVAErrorFraction_min = iTMVAErrorFraction_min; }
   void   setTMVAThetaCutVariable( bool iB = false ) { fTMVAThetaCutVariableSet = iB; }
   void   setTMVAMethod( string iMethodName = "BDT", unsigned int iMethodCounter = 0 );

   ClassDef(VTMVAEvaluator, 18 );
};

#endif
