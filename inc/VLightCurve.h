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
#include "VPlotUtilities.h"

#include "TArrow.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TLine.h"

using namespace std;

class VLightCurve : public VPlotUtilities
{
   private:

   string fAnaSumFile;
   double fDayInterval;

   string fDataType;           // possible data types are TeV_anasum, TeV_ascii, XRT_ascii

   VFluxCalculation*           fFluxCombined;     // does this has to be global? used in initializeTeVLightCurve() only
   vector< VLightCurveData* >  fFluxInterval;
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

// phases
   double   fPhase_MJD0;
   double   fPhase_Period_days;
   bool     fPhasePlotting;

// plotting
   TCanvas *fCanvasLightCurve;
   double   fPlottingMJDMin;
   double   fPlottingMJDMax;
   TGraphAsymmErrors *fLightCurveGraph;

   double   fRateAxisMin;
   double   fRateAxisMax;
   string   fRateAxisTitle;

// private functions
   bool     fillTeV_anasum( bool iPrint );
   bool     fillTeV_ascii(  bool iPrint );
   bool     fillXRT_ascii(  bool iPrint );

   double   getLightCurveAxisRange_Min();
   double   getLightCurveAxisRange_Max();
   double   getPhase( double iMJD );

   public:

   VLightCurve();
  ~VLightCurve() {}
   bool     fill( double iEMin_TeV = 1., double iEMax_TeV = -1., bool iPrint = false );       // energy [TeV]
   string   getDataType() { return fDataType; }
   vector< VLightCurveData* > getLightCurveData();
   TGraphAsymmErrors*         getLightCurveGraph() { return fLightCurveGraph; }
   string   getLightCurveAxisTitle();
   string   getRateAxisTitle() { return fRateAxisTitle; }
   bool     initializeTeVLightCurve( string iASCIIFile );
   bool     initializeTeVLightCurve( string iAnaSumFile, double iDayInterval, double iMJDMin = -1., double iMJDMax = -1. );
   bool     initializeXRTLightCurve( string iXRTFile, double iMJDStart = 54857.09977457897 );
   void     printLightCurve( bool bFullDetail = true );
   TCanvas* plotLightCurve( TCanvas* iCanvasLightCurve = 0, string iCanvasName = "cL", int iPlotConfidenceInterval = 0, string iPlottingOption = "p" );
   bool     plotObservingPeriods( TCanvas* iCanvasLightCurve, string iDataFile, int iColor );
   void     setPhaseFoldingValues( double iZeroPhase_MJD = -99., double iPhase_Days =99., bool bPlotPhase = true );
   void     setPlottingParameter( double iPlottingMJDMin, double iPlottingMJDMax );
   void     setSignificanceParameters( double iThresholdSignificance = -9999., double iMinEvents = -9999., double iUpperLimit = 0.99, int iUpperlimitMethod = 0, int iLiMaEqu = 17 );
   void     setLightCurveAxis( double iYmin = -9.e10, double iYmax = -9.e10, string iAxisTitle = "tevRate" );
   void     setSpectralParameters( double iMinEnergy = 0., double E0 = 1., double alpha = -2.5 );

   ClassDef( VLightCurve, 1 );
};

#endif
