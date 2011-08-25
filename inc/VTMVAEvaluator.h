//! VTMVAEvaluator use a TMVA weight file for energy dependent gamma/hadron separation
// Revision $Id: VTMVAEvaluator.h,v 1.1.2.3 2011/04/07 15:27:22 gmaier Exp $

#ifndef  VTMVAEvaluator_H
#define  VTMVAEvaluator_H

#include "CData.h"
#include "VMathsandFunctions.h"
#include "VPlotUtilities.h"
#include "VTMVARunData.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
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

   bool     fTMVAIgnoreTheta2Cut;           // ignore theta2 cut in TMVA

   double   fSignalEfficiency;

   double   fSpectralIndexForEnergyWeighting;        // used to calculate the spectral weighted mean of an energy bin

// cut values (energy dependent)
   vector< double > fBoxCutValue_theta2;

   vector< vector< Double_t > > fBoxCutValue_min;
   vector< vector< Double_t > > fBoxCutValue_max;
   vector< vector< string > > fBoxCutValue_Name;

// gamma/hadron separation variables
   float    fNImages; 
   float    fMSCW;
   float    fMSCL;
   float    fMWR;
   float    fMLR;
   float    fEmissionHeight;
   float    fEChi2;
   float    fdE;
   float    fTheta2;

   vector< string > getTrainingVariables( string );
   void     reset();

   public:

   VTMVAEvaluator();
  ~VTMVAEvaluator() {};

   double evaluate( double, double );
   double getBoxCut_Theta2( double iEnergy_log10TeV );
   TGraph* getBoxCut_Theta2_Graph();
   vector< double > getBoxCut_Theta2() { return fBoxCutValue_theta2; }
   unsigned int getSpectralWeightedEnergyBin();
   bool   initializeWeightFiles( string iWeightFileName, unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max );
   bool   initializeDataStrutures( CData* iC );
   bool   IsZombie() { return fIsZombie; }
   void   plotBoxCuts();
   void   setDebug( bool iB = false ) { fDebug = iB; }
   void   setIgnoreTheta2Cut( bool iB = false ) { fTMVAIgnoreTheta2Cut = iB; }
   void   setSignalEfficiency( double iE = 0.5 ) { fSignalEfficiency = iE; }
   void   setSpectralIndexForEnergyWeighting( double iS = -2. )  { fSpectralIndexForEnergyWeighting = iS; }

   ClassDef(VTMVAEvaluator, 2 );
};

#endif
