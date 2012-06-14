//! VLightCurve  light curves
// Revision $Id: VLightCurve.h,v 1.1.2.2 2011/06/16 14:53:04 gmaier Exp $

#ifndef VLightCurve_H
#define VLightCurve_H

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "VFluxCalculation.h"
#include "VLightCurveData.h"
#include "VLightCurveUtilities.h"
#include "VPlotUtilities.h"

#include "TArrow.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLine.h"
#include "TProfile.h"
#include "TRandom.h"

using namespace std;

class VLightCurve : public VPlotUtilities, public VLightCurveUtilities
{
   private:

   string fName;

   string fAnaSumFile;
   double fDayInterval;

   string fDataType;           // possible data types are TeV_anasum, TeV_ascii, XRT_ascii

   VFluxCalculation*           fFluxCombined;     // does this has to be global? used in initializeTeVLightCurve() only

   double                      fEnergy_min_TeV;
   double                      fEnergy_max_TeV;

// spectral parameters (assuming power law)
   double fMinEnergy;   //!< calculate flux limit above this energy [TeV]
   double fMaxEnergy;   //!< maximum energy to be taken into account [TeV]
   double fE0;          //!< calculate flux at this energy [TeV]
   double fAlpha;       //!< assumed spectral index

// significance and upper flux limit parameters
   int    fLiMaEqu;
   double fThresholdSignificance;
   double fMinEvents;
   double fUpperLimit;
   int    fUpperLimitMethod;

// light curve filling
   TH1D    *fObservingInvervallHisto;

// plotting
   TCanvas *fCanvasLightCurve;
   double   fPlottingMJDMin;
   double   fPlottingMJDMax;
   TGraphAsymmErrors *fLightCurveGraph;
   TH2D*    fMCRandomizedPhaseogram;
   TProfile* fMCRandomizedPhaseogramProf;

   double   fRateAxisMin;
   double   fRateAxisMax;
   string   fRateAxisTitle;
   bool     fRateAxisTitleUnSet;

// private functions
   bool     fillTeV_anasum( bool iPrint );
   bool     fillTeV_ascii(  bool iPrint );
   bool     fillXRT_ascii(  bool iPrint );

   double   getLightCurveAxisRange_Min();
   double   getLightCurveAxisRange_Max();

   public:

   VLightCurve();
  ~VLightCurve() {}
   bool     fill( double iEMin_TeV = 1., double iEMax_TeV = -1., bool iPrint = false );       // energy [TeV]
   string   getDataType() { return fDataType; }
   TGraphAsymmErrors*         getLightCurveGraph() { return fLightCurveGraph; }
   string   getLightCurveAxisTitle();
   TH2D*    getRandomizedPhaseogram()      { return fMCRandomizedPhaseogram; }
   TProfile* getRandomizedPhaseogramProf() { return fMCRandomizedPhaseogramProf; }
   TH1D*    fillObservingIntervallHistogram( bool bPlot = false, double iPlotMax = 10., string iName = "hIntervalls" );
   bool     fillLightCurveMCPhaseFolded( string iOutFile, double iGapsToFill_days = 20., double iPhaseBinning = 0.025, bool bPlotDebug = false );
   bool     fillRandomizedPhaseogram( double iMCCycles, double iPhaseError_low, double iPhaseErrorUp, string iHisName, double iHisMin_y, double iHisMax_y );
   string   getRateAxisTitle() { return fRateAxisTitle; }
   bool     initializeTeVLightCurve( string iASCIIFile );
   bool     initializeTeVLightCurve( string iAnaSumFile, double iDayInterval, double iMJDMin = -1., double iMJDMax = -1. );
   bool     initializeXRTLightCurve( string iXRTFile, double iMJDMin = -1., double iMJDMax = -1. );
   TCanvas* plotLightCurve( TCanvas* iCanvasLightCurve = 0, string iCanvasName = "cL", int iPlotConfidenceInterval = -1, string iPlottingOption = "p" );
   bool     plotObservingPeriods( TCanvas* iCanvasLightCurve, string iDataFile, int iColor );
   void     setPlottingParameter( double iPlottingMJDMin, double iPlottingMJDMax );
   void     setSignificanceParameters( double iThresholdSignificance = -9999., double iMinEvents = -9999., double iUpperLimit = 0.99, int iUpperlimitMethod = 0, int iLiMaEqu = 17 );
   void     setLightCurveAxis( double iYmin = -9.e10, double iYmax = -9.e10, string iAxisTitle = "tevRate" );
   void     setName( string iName ) { fName = iName; }
   void     setSpectralParameters( double iMinEnergy = 0., double E0 = 1., double alpha = -2.5 );

   ClassDef( VLightCurve, 3 );
};

#endif
