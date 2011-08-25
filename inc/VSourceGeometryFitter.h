//! VSourceGeometryFitter analyse source geometry (position and extension)
// Revision $Id: VSourceGeometryFitter.h,v 1.1.2.3 2011/03/28 03:45:10 ealiu Exp $

#ifndef VSourceGeometryFitter_H
#define VSourceGeometryFitter_H

#include "VAnalysisUtilities.h"
#include "VPlotUtilities.h"

#include "TCanvas.h"
#include "TEllipse.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH2D.h"
#include "TLine.h"
#include "TMath.h"
#include "Minuit2/FCNBase.h"
#include "TFitterMinuit.h"
#include "TMinuit.h"

#include <iostream>
#include <string>

using namespace std;


///////////////////////////////////////////////////////////////////////////////
class VSourceGeometryFitterData
{
   public:

   string           fFitterName;
   vector< string > fParameterName;
   vector< double > fParameterInitValue;
   vector< double > fParameterStep;
   vector< double > fParameterLowerLimit;
   vector< double > fParameterUpperLimit;
   int              fFitResult_Status;
   vector< double > fFitResult_Parameter;
   vector< double > fFitResult_ParameterError;

   VSourceGeometryFitterData();
  ~VSourceGeometryFitterData() {}
};

class VSourceGeometryFitter : public VAnalysisUtilities, public VPlotUtilities
{
   private:

   bool   fDebug;

   string fAnasumDataFile;
   int    fRunNumber;
   double fXStart;
   double fYStart;

// sky map to be fitted
   TH2D *fHisSkyMap;

// default fitter data
   vector< VSourceGeometryFitterData* > fDefaultFitterData;

// fitter used
   VSourceGeometryFitterData            *fFitter;

   void setFitterDefaultData();

   public:

   VSourceGeometryFitter();
   VSourceGeometryFitter( string iAnaSumDataFile, int irun = -1 );
  ~VSourceGeometryFitter() {}

   void     fitSource( string iHisName = "hmap_stereoUC_diff", double xStart = 0., double yStart = 0., double xyRange = 0.15 );
   TH2D*    getSkyMap() { return fHisSkyMap; }
   void     help();
   TCanvas* plot( double rmax = 0.2, double zmin = -1000., double zmax = -1000., string iPlotMode = "colz" );
   void     plotFitResult();
   TGraph*  plotSourceGeometry( int iColor = 1 );
   void     setDebug( bool iB = true ) { fDebug = iB; }
   bool     setFitter( string iFitter );

   ClassDef(VSourceGeometryFitter,1);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Functions for source position and extension fitting
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// radial symmetric source
///////////////////////////////////////////////////////////////////////////////
class VFun_SourceDescription_RadialSymmetricSource_Chi2 : public ROOT::Minuit2::FCNBase
{
   private:

   TH2D *hSkyMap;
   double xmin;
   double xmax;
   double ymin;
   double ymax;

   public:

   VFun_SourceDescription_RadialSymmetricSource_Chi2( TH2D *iSkymap = 0, double i_xmin = -1., double i_xmax = 1., double i_ymin = -1., double i_ymax = 1. );

/////////////////////////////////
// function to be minimized
   double operator() ( const std::vector<double> & par ) const 
   {
// check size fo parameter vector
       if( par.size() != 5 )
       {
          cout << "VFun_SourceDescription_RadialSymmetricSource_Chi2: error in parameter vector size; expect 5, is " << par.size() << endl;
	  return 0.;
       }

// set variables
       double x = 0.;
       double y = 0.;

       double sum = 0.;
       double fT = 0.;
       double fH = 0.;

       double t2 = 0.;
       double sigmaSource2 = par[2]*par[2];

// loop over sky map
       if( hSkyMap )
       {
	  int nbinsX = hSkyMap->GetNbinsX();
	  int nbinsY = hSkyMap->GetNbinsY();
	  for( int i = 1; i <= nbinsX; i++ )
	  {
	      x = hSkyMap->GetXaxis()->GetBinCenter( i );
// check x-range
	      if( x > xmax ) continue;
      	      if( x < xmin ) continue;
	      for( int j = 1; j <= nbinsY; j++ )
	      {
		  y = hSkyMap->GetYaxis()->GetBinCenter( j );
// check y-range
		  if( y > ymax ) continue;
		  if( y < ymin ) continue;

// skip empty bins
		  if( hSkyMap->GetBinContent( i, j ) <= 0. ) continue;

// calculate theta2
		  t2 = (x-par[3])*(x-par[3]) + (y-par[4])*(y-par[4]);

// calculate expectation from model function
		  fT = par[0] + par[1] * TMath::Exp( -1.*t2 / 2. / sigmaSource2 );
		  if( isnan( fT ) ) continue;

// get value and error in histogram
		  fH = hSkyMap->GetBinContent( i, j );

// calculate chi2
		  if( fH != 0. && fH > -90. ) sum += ( fT - fH ) * ( fT - fH ) / fH;
	       }
	   }
       }
       return sum;
   }
   double Up() const { return 1.; }
};

///////////////////////////////////////////////////////////////////////////////
// 2D Gauss
//
// TODO: needs more work to take zero and negative bins into account
// 
///////////////////////////////////////////////////////////////////////////////
class VFun_SourceDescription_2DNormal_LL: public ROOT::Minuit2::FCNBase
{
   private:

   TH2D *hSkyMap;
   double xmin;
   double xmax;
   double ymin;
   double ymax;

   public:

   VFun_SourceDescription_2DNormal_LL( TH2D *iSkymap = 0, double i_xmin = -1., double i_xmax = 1., double i_ymin = -1., double i_ymax = 1. );

/////////////////////////////////
// function to be minimized
   double operator() ( const std::vector<double> & par ) const 
   {
// check size fo parameter vector
       if( par.size() != 5 )
       {
          cout << "VFun_SourceDescription_2DNormal: error in parameter vector size; expect 5, is " << par.size() << endl;
       }

// initialize variables
       double LL = 0.;
       double sum = 0.;
       double  rho =  par[0];
       double  meanX = par[1];
       double  sigmaX = par[2];
       double  meanY = par[3];
       double  sigmaY = par[4];

       double x = 0.;
       double y = 0.;
       double n = 0.;                                // measured sum in channel i

       if( rho * rho < 1. && sigmaX > 0. && sigmaY > 0. )
       {
	   int nbinsX = hSkyMap->GetNbinsX();
	   int nbinsY = hSkyMap->GetNbinsY();
	   for( int i = 1; i <= nbinsX; i++ )
	   {
	       x = hSkyMap->GetXaxis()->GetBinCenter( i );
// check x-range
	       if( x > xmax ) continue;
      	       if( x < xmin ) continue;
	       for( int j = 1; j <= nbinsY; j++ )
	       {
		   y = hSkyMap->GetYaxis()->GetBinCenter( j );
// check y-range
		   if( y > ymax ) continue;
		   if( y < ymin ) continue;

		   n = hSkyMap->GetBinContent( i, j );

// check for valid entries
		   if( n > -999. )
		   {
// calculate log-likelihood
		       sum  = ( x - meanX ) * ( x - meanX ) / sigmaX / sigmaX;
		       sum += ( y - meanY ) * ( y - meanY ) / sigmaY / sigmaY;
		       sum += -2. * rho * ( x - meanX ) / sigmaX * ( y - meanY ) / sigmaY;
		       sum *= -1. / 2. / ( 1. - rho * rho );
		       sum  = exp( sum );
		       sum *= 1. / 2. / M_PI / sigmaX / sigmaY / sqrt( 1. - rho * rho );

// assume Poisson fluctuations (neglecting background noise)
		       if( n > 0. && sum > 0. )    LL += n * log(sum) - sum - n * log(n) + n;
		       else LL += -1. * sum;
		   }
	       }
	   }
       }
       return -1. * LL;
    }

   double Up() const { return 1.; }
};



#endif
