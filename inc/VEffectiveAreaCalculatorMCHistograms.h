//!  VEffectiveAreaCalculatorMCHistograms filling, reading, writing of MC histograms for effective area calculation
// Revision $Id: VEffectiveAreaCalculatorMCHistograms.h,v 1.1.2.2.4.2 2011/02/03 12:49:08 gmaier Exp $

#ifndef VEffectiveAreaCalculatorMCHistograms_H
#define VEffectiveAreaCalculatorMCHistograms_H

#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "TProfile.h"
#include "TTree.h"

#include "VGammaHadronCuts.h"
#include "VSpectralWeight.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VEffectiveAreaCalculatorMCHistograms : public TNamed
{
   private:

// cuts
   VGammaHadronCuts *fCuts;                                     //! don't write cuts to output file (preliminary)

// spectral weight calculator
   VSpectralWeight *fSpectralWeight;

   public:

// azimuth and spectral index bins
   vector< double > fVMinAz;
   vector< double > fVMaxAz;
   vector< double > fVSpectralIndex;

// MC histograms
   vector< vector< TH1D* > > hVEmc;                     // [spectral index][az]
   vector< vector< TProfile* > > hVEmcSWeight;          // [spectral index][az]

   VEffectiveAreaCalculatorMCHistograms();
  ~VEffectiveAreaCalculatorMCHistograms() {}

   bool      add();
   bool      fill( double i_ze, TChain *i_MCData );
   VGammaHadronCuts* getAnaCuts() { return fCuts; }
   TH1D*     getHistogram_Emc( unsigned int iAz, unsigned int iIndex );
   TProfile* getHistogram_EmcWeight( unsigned int iAz, unsigned int iIndex );
   void      initializeHistograms( vector< double > iAzMin, vector< double > iAzMax, vector< double > iSpectralIndex, int nbins = 60, double iMin = -2., double iMax = 4. );
   void      listEntries();
   bool      readFromEffectiveAreaTree( string iFile );
   bool      readFromEffectiveAreaFile( string iFile );
   void      setCuts( VGammaHadronCuts* iAnaCuts ) { fCuts = iAnaCuts; }
   bool      setMonteCarloEnergyRange( double iMin, double iMax, double iMCIndex = 2. );

   ClassDef( VEffectiveAreaCalculatorMCHistograms, 3 );
};

#endif
