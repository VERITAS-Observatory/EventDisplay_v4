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

   float f_pput_min;
   float f_pput_max;

   float f_pput_Energy_linTeV_min;
   float f_pput_Energy_linTeV_max;

   float f_plot_alt_min;
   float f_plot_alt_max;

   void getMergedFigureOfMerits( VSiteData *iSite, float* fom, float* fom_error, string iDirectionString = "" );

   public:

   VPlotPPUT();
  ~VPlotPPUT() {}

   void plot( bool bSouth, string iDataList, bool bPlotPredictions = false );
   void setDebug( bool iB = false ) { fDebug = iB; }
   void setPlotAltitudeRange( float alt_min = 410., float alt_max = 4000. ) { f_plot_alt_min = alt_min; f_plot_alt_max = alt_max; }
   void setPPUTEnergyRange( float pput_Energy_linTeV_min = -99., float pput_Energy_linTeV_max = -99. ) { f_pput_Energy_linTeV_min = pput_Energy_linTeV_min; 
                                                                                                         f_pput_Energy_linTeV_max = pput_Energy_linTeV_max; }
   void setPPUTRange( float pput_min = 0.6, float pput_max = 2.0 );
};

#endif
