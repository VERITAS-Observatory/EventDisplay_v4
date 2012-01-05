//! VTMVAEvaluator use a TMVA weight file for energy dependent gamma/hadron separation
// Revision $Id: VTMVAEvaluator.h,v 1.1.2.3 2011/04/07 15:27:22 gmaier Exp $

#ifndef  VTMVAEvaluator_H
#define  VTMVAEvaluator_H

#include "CData.h"
#include "VMathsandFunctions.h"
#include "VPlotUtilities.h"
#include "VStatistics.h"
#include "VTMVARunData.h"

#include <fstream>
#include <iostream>
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

class VTMVAEvaluator : public TNamed, public VPlotUtilities
{
   private:

   bool     fDebug;
   bool     fIsZombie;

   CData*   fData;
   vector< TMVA::Reader* > fTMVAReader;
   vector< double >        fEnergyCut_Log10TeV_min;
   vector< double >        fEnergyCut_Log10TeV_max;
   vector< unsigned int >  fEnergyReconstructionMethod;
   vector< TString >       fTMVAMethodTag;
   vector< double >        fSignalEfficiency;                // from user or best signal/sqrt(noise)
   vector< double >        fBackgroundEfficiency;            // from best signal/sqrt(noise)
   double                  fSignalEfficiencyNoVec;
   vector< double >        fTMVACutValue;
   double                  fTMVACutValueNoVec;

   string                  fParticleNumberFileName;          // particle numbers are read from this file
   double                  fOptmizationSourceStrengthCrabUnits; 
   double                  fOptmizationMinBackGroundEvents;
   double                  fOptimizationBackgroundAlpha;
   double                  fOptimizationObservingTime_h;

   bool     fTMVAIgnoreTheta2Cut;           // ignore theta2 cut in TMVA
   bool     fTMVAThetaCutVariableSet;       // check if TMVA provides a theta2 cut variable

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
   float    fDummy;

   bool     bPlotEfficiencyPlotsPerEnergy;

   double           getTMVACutValueFromSignalEfficiency( double iSignalEfficiency, unsigned int iBin, string iWeightFileName );
   bool             optimizeSensitivity( unsigned int i, string iTMVARootFile );
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

   double evaluate();
   double getBoxCut_Theta2( double iEnergy_log10TeV );
   TGraph* getBoxCut_Theta_Graph();
   TGraph* getBoxCut_Theta2_Graph();
   vector< double > getBoxCut_Theta2() { return fBoxCutValue_theta2; }
   unsigned int getSpectralWeightedEnergyBin();
   bool   getTMVAThetaCutVariable() { return fTMVAThetaCutVariableSet; }
   bool   initializeWeightFiles( string iWeightFileName, unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max );
   bool   initializeDataStrutures( CData* iC );
   bool   isBoxCuts() { return fTMVAMethodName_BOXCUTS; }
   bool   IsZombie() { return fIsZombie; }
   void   plotBoxCuts();
   void   plotSignalAndBackgroundEfficiencies( bool iLogY = true, double iYmin = 1.e-4 );
   void   printSignalEfficiency();
   void   setDebug( bool iB = false ) { fDebug = iB; }
   void   setIgnoreTheta2Cut( bool iB = false ) { fTMVAIgnoreTheta2Cut = iB; }
   void   setSensitivityOptimizationParameters( double iSourceStrength = 0.001, double iMinBackgroundEvents = 0., double iBackgroundAlpha = 1./5.,
                                                double iObservationTime_h = 50. )
          { fOptmizationSourceStrengthCrabUnits = iSourceStrength; 
	    fOptmizationMinBackGroundEvents = iMinBackgroundEvents; 
	    fOptimizationBackgroundAlpha = iBackgroundAlpha;
	    fOptimizationObservingTime_h = iObservationTime_h; }
   void   setParticleNumberFile( string iParticleNumberFile = "" ) { fParticleNumberFileName = iParticleNumberFile; }
   void   setPlotEffiencyPlotsPerEnergy( bool iB = false ) { bPlotEfficiencyPlotsPerEnergy = iB; }
   void   setSignalEfficiency( double iE = -99. );
   void   setSpectralIndexForEnergyWeighting( double iS = -2. )  { fSpectralIndexForEnergyWeighting = iS; }
   void   setTMVACutValue( double iE = -99. );
   void   setTMVAErrorFraction( double iTMVAErrorFraction_min = 0.2 ) { fTMVAErrorFraction_min = iTMVAErrorFraction_min; }
   void   setTMVAThetaCutVariable( bool iB = false ) { fTMVAThetaCutVariableSet = iB; }
   void   setTMVAMethod( string iMethodName = "BDT", unsigned int iMethodCounter = 0 );

   ClassDef(VTMVAEvaluator, 9 );
};

#endif
