//! VHistogramUtilities.h  utility class to manipulate histograms
// Revision $Id: VHistogramUtilities.h,v 1.1.2.3 2011/02/18 21:01:30 gmaier Exp $

#ifndef VHistogramUtilities_H
#define VHistogramUtilities_H

#include "TDirectory.h"
#include "TF1.h"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VHistogramUtilities
{
    private:

    bool fDebug;


    public:

    VHistogramUtilities();
   ~VHistogramUtilities() {}

   TH1D*            get_Bin_Distribution( TH2D *h, int ion, double rmax, double rSource, bool iDiff, TH2D *hTest );
   TH1D*            get_Cumulative_Histogram( TH1D* iH_in, bool iNormalize, bool iLeft_to_right );
   bool             get_Graph_from_Histogram( TH1F* h, TGraphErrors* g, bool bIgnoreErrors = false );
   bool             get_Graph_from_Histogram( TH1F* h, TGraphAsymmErrors* g, bool bIgnoreErrors = false, bool bLinXaxis = false );
   TGraphErrors*    get_Profile_from_TH2D( TH2D *iH, TGraphErrors*g = 0, string iM = "median", int i_rebin = 2, double iXaxisValue = -10. );
   static TH1D*     get_ResidualHistogram_from_TF1( string iname = "", TH1D *h = 0, TF1 *f = 0 );

   TH1F*            get_CTA_IRF_Histograms( string iHistogramName, double iCameraOffset );

    void            setDebug( bool iB = true ) { fDebug = iB; }
};

#endif

