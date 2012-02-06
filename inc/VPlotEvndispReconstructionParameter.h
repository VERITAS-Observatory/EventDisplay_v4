//! VPlotEvndispReconstructionParameter plot eventdisplay reconstruction parameters depending on telescopes
// Revision $Id$

#ifndef VPlotEvndispReconstructionParameter_H
#define VPlotEvndispReconstructionParameter_H

#include "TCanvas.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"

#include "Cshowerpars.h"
#include "Ctpars.h"
#include "VEvndispReconstructionParameter.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VPlotEvndispReconstructionParameter : public TNamed 
{
   private:

   bool  fDebug;

   string                           fDataFileName;
   TChain                          *fDataChain;
   TFile                           *fDataFile;
   Cshowerpars                     *fDataShowerPars;
   Ctpars                     *fDataTpars;
   VEvndispReconstructionParameter *fEvndispReconstructionParameter;

   vector< string > fEvndispReconstructionParameterName;
   vector< TH1D* >  fEvndispReconstructionParameterHisto;
   vector< TH1D* >  fEvndispReconstructionParameterHistoInt;

   TCanvas *fPlotCanvas;

   void reset();
   bool fill( unsigned int iMethod, unsigned int iTelescope, int iTelescopeTypeCounter = 0 );

   public:

   VPlotEvndispReconstructionParameter();
  ~VPlotEvndispReconstructionParameter() {};

   bool initialize( string iEventdisplayFileName,
                    int iNEnergyBins = 50, double iEnergy_TeV_log_min = -2., double iEnergy_TeV_log_max = 2.5 );
   void plot( unsigned int iMethod, unsigned int iTelescope, int iTelescopeTypeCounter = 0 );
   void setDebug( bool iB = false ) { fDebug = iB; }

   ClassDef(VPlotEvndispReconstructionParameter,1);
};

#endif
