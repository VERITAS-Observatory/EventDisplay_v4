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

   string                  fParticleNumberFileName;          // particle numbers are read from this file
   double                  fOptmizationSourceStrengthCrabUnits; 
   double                  fOptmizationMinBackGroundEvents;

   bool     fTMVAIgnoreTheta2Cut;           // ignore theta2 cut in TMVA
   bool     fTMVAThetaCutVariableSet;       // check if TMVA provides a theta2 cut variable

   string   fTMVAMethodName;
   unsigned int fTMVAMethodCounter;

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
   float    fTheta2;

   bool     bPlotEfficiencyPlotsPerEnergy;

   bool             optimizeSensitivity( unsigned int i, string iTMVARootFile );
   vector< string > getTrainingVariables( string );
   void             plotEfficiencyPlotsPerEnergy( unsigned int iBin, 
                                                  TGraph* iGSignal_to_sqrtNoise, TH1F* iHSignal_to_sqrtNoise, 
						  TH1F* hEffS, TH1F* hEffB, 
						  double iEnergy_Log10TeV_min, double iEnergy_Log10TeV_max );
   void     reset();

   public:

   VTMVAEvaluator();
  ~VTMVAEvaluator() {};

   double evaluate( double, double );
   double getBoxCut_Theta2( double iEnergy_log10TeV );
   TGraph* getBoxCut_Theta_Graph();
   TGraph* getBoxCut_Theta2_Graph();
   vector< double > getBoxCut_Theta2() { return fBoxCutValue_theta2; }
   unsigned int getSpectralWeightedEnergyBin();
   bool   getTMVAThetaCutVariable() { return fTMVAThetaCutVariableSet; }
   bool   initializeWeightFiles( string iWeightFileName, unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max );
   bool   initializeDataStrutures( CData* iC );
   bool   IsZombie() { return fIsZombie; }
   void   plotBoxCuts();
   void   plotSignalAndBackgroundEfficiencies( bool iLogY = true, double iYmin = 1.e-4 );
   void   printSignalEfficiency();
   void   setDebug( bool iB = false ) { fDebug = iB; }
   void   setIgnoreTheta2Cut( bool iB = false ) { fTMVAIgnoreTheta2Cut = iB; }
   void   setSensitivityOptimizationParameters( double iSourceStrength = 0.001, double iMinBackgroundEvents = 5. )
          { fOptmizationSourceStrengthCrabUnits = iSourceStrength; fOptmizationMinBackGroundEvents = iMinBackgroundEvents; }
   void   setParticleNumberFile( string iParticleNumberFile = "" ) { fParticleNumberFileName = iParticleNumberFile; }
   void   setPlotEffiencyPlotsPerEnergy( bool iB = false ) { bPlotEfficiencyPlotsPerEnergy = iB; }
   void   setSignalEfficiency( double iE = 0.5 );
   void   setSpectralIndexForEnergyWeighting( double iS = -2. )  { fSpectralIndexForEnergyWeighting = iS; }
   void   setTMVAThetaCutVariable( bool iB = false ) { fTMVAThetaCutVariableSet = iB; }
   void   setTMVAFileParameters( string iMethodName = "Method_Cuts", unsigned int iMethodCounter = 0 )
          { fTMVAMethodName = iMethodName; fTMVAMethodCounter = iMethodCounter; }

   ClassDef(VTMVAEvaluator, 4 );
};

#endif
