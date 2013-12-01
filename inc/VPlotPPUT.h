//! VPlotPPUT  plot PPUT vs asl and magnetic field

#ifndef VPlotPPUT_H
#define VPlotPPUT_H

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TF2.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TGraph2DErrors.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLine.h"

#include "VPlotWPPhysSensitivity.h"
#include "VSiteData.h"

using namespace std;

class VPlotPPUT
{
   private:

   bool fDebug;

   void getMergedFigureOfMerits( VSiteData *iSite, float* fom, float* fom_error, string iDirectionString = "" );

   public:

   VPlotPPUT();
  ~VPlotPPUT() {}

   void plot( bool bSouth, string iDataList );
   void setDebug( bool iB = false ) { fDebug = iB; }
};

#endif
