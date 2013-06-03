//! VPlotRadialAcceptance plot radial acceptance curves
// 

#ifndef VPlotRadialAcceptance_H
#define VPlotRadialAcceptance_H

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1D.h"
#include "TLine.h"
#include "TText.h"

#include <iostream>
#include <string>

#include "VHistogramUtilities.h"
#include "VPlotUtilities.h"
#include "VUtilities.h"

using namespace std;

class VPlotRadialAcceptance : public VPlotUtilities
{
   private:

   bool fDebug;

   string fName;

   TFile* fAcceptanceFile;
   TH1D*  fAcceptanceHisto;
   TH1D*  fAcceptanceHistoFit;
   TF1*   fAcceptanceFunction;

   double fAxis_x_min;
   double fAxis_x_max;
   double fAxis_y_min;
   double fAxis_y_max;

   public:

   VPlotRadialAcceptance( string iFile = "" );
  ~VPlotRadialAcceptance() {}

   TF1*  getAcceptanceFunction() { return fAcceptanceFunction; }
   TH1D* getAcceptanceHisto()    { return fAcceptanceHisto;    }
   TH1D* getAcceptanceHistoFit() { return fAcceptanceHistoFit; }

   TCanvas* plot( TCanvas *cX = 0 );
   TCanvas* plotResiduals( TCanvas *cX = 0, double i_res_min = -0.5, double i_res_max = 0.5, bool iDrawChi2 = true );
   bool     openAcceptanceFile( string iFile, unsigned int iZeBin = 0 );
   void     setAxisRange( double x_min = 0., double x_max = 3.5, double y_min = 0., double y_max = 1.5 );
   void     setName( string iName ) { fName = iName; }
   
  ClassDef( VPlotRadialAcceptance, 1 );
};

#endif
