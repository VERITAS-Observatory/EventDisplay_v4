//! VPlotEvndispReconstructionParameter plot eventdisplay reconstruction parameters depending on telescopes
// Revision $Id$

#ifndef VPlotEvndispReconstructionParameter_H
#define VPlotEvndispReconstructionParameter_H

#include "TCanvas.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"

#include "Cshowerpars.h"
#include "CtparsShort.h"
#include "VDetectorGeometry.h"
#include "VDetectorTree.h"
#include "VEvndispReconstructionParameter.h"

#include <iostream>
#include <vector>

using namespace std;

class VPlotEvndispReconstructionParameter : public TObject
{
   private:

   bool  fDebug;

   string                           fDataFileName;
   TChain                          *fDataChain;
   TFile                           *fDataFile;
   Cshowerpars                     *fDataShowerPars;
   CtparsShort                     *fDataTpars;
   VEvndispReconstructionParameter *fEvndispReconstructionParameter;
   VDetectorGeometry               *fDetectorGeometry;

   vector< string > fEvndispReconstructionParameterName;
   vector< TH1D* >  fEvndispReconstructionParameterHisto;
   vector< TH1D* >  fEvndispReconstructionParameterHistoInt;

   TCanvas *fPlotCanvas;

   void reset();
   bool fill( unsigned int iMethod, unsigned int iTelescope );

   public:

   VPlotEvndispReconstructionParameter();
  ~VPlotEvndispReconstructionParameter() {};

   bool initialize( string iEventdisplayFileName,
                    int iNEnergyBins = 50, double iEnergy_TeV_log_min = -2., double iEnergy_TeV_log_max = 2. );
   void plot( unsigned int iMethod, unsigned int iTelescope );
   void setDebug( bool iB = false ) { fDebug = iB; }

   ClassDef(VPlotEvndispReconstructionParameter,1);
};

#endif
