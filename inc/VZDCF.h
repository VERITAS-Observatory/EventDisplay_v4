//! VZDCF (plotting) functions for Z-transformed Discrete Correlation Function algorithm (ZDCF)
// Revision $Id$

#ifndef VZDCF_H
#define VZDCF_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "VPlotUtilities.h"
#include "VZDCFData.h"

#include "TBox.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TLine.h"

using namespace std;

class VZDCF : public VPlotUtilities
{
   private:

// 1 sigma interval and peak
   double fMLPeakposition;
   double fML1Sigmainterval_low;
   double fML1Sigmainterval_up;

   vector< VZDCFData* > fZDCFData;

   double getZDCFData_dcf_min( bool bError = false );
   double getZDCFData_dcf_max( bool bError = false );
   double getZDCFData_tau_min( bool bError = false );
   double getZDCFData_tau_max( bool bError = false );

   void   plot( TCanvas *c = 0, bool bZDCF = true );

   public:

   VZDCF();
  ~VZDCF() {}

   void plotZDCF( TCanvas *c = 0 );
   void plotZDCFoverError( TCanvas *c = 0 );
   bool print();
   bool readZDCF( string iFile );
   void setMLinterval( double iMLPeak = -9999., double iML1Sigma_low = -99999., double iML1Sigma_up = -99999. );

   ClassDef( VZDCF, 1 );

};

#endif

