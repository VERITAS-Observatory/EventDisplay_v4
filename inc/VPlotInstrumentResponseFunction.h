//! VPlotInstrumentResponseFunction effective area plotter
// Revision $Id: VPlotInstrumentResponseFunction.h,v 1.1.2.8 2011/03/28 07:05:52 gmaier Exp $

#ifndef VPlotInstrumentResponseFunction_H
#define VPlotInstrumentResponseFunction_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TH1D.h"
#include "TLine.h"
#include "TMath.h"
#include "TText.h"

#include "VHistogramUtilities.h"
#include "VInstrumentResponseFunctionReader.h"
#include "VPlotUtilities.h"

using namespace std;

class VPlotInstrumentResponseFunction : public VPlotUtilities, public VHistogramUtilities
{
    private:

    bool   fDebug;
    string fName;

// effective area data
    vector< VInstrumentResponseFunctionReader* > fData;

// resolution fitter
    string fFunction_fitResolution;
    double fXmin_fitResolution;
    double fXmax_fitResolution;
    TF1*   fTF1_fitResolution;

// last plotted graph
    TGraphErrors *gLastPlottedGraph;

// canvas size
    int    fCanvasSize_X;
    int    fCanvasSize_Y;

// general resolution plotter
    TCanvas* plotResolution( string iName, string iCanvasTitle, string iYTitle,
                             double iYmin, double iYmax, string iResolutionTreeName, string iXaxis );
    TCanvas* plotResolution2D( unsigned int iDataSetID, string iName, string iCanvasTitle, string iYTitle,
                               double iYmin, double iYmax, string iResolutionTreeName, string iXaxis, double iEnergySlice_GeV = -1. );

    public:

    VPlotInstrumentResponseFunction();
   ~VPlotInstrumentResponseFunction() {}

    bool         addInstrumentResponseData( int iDataID, string iFileList );
    bool         addInstrumentResponseData( string iInstrumentResponseFile, string iA_MC );
    bool         addInstrumentResponseData( string iInstrumentResponseFile,
    				            double iZe = 20., double iWoff = 0.5, int iAzBin = 0,
					    double iIndex = 2.4, int iNoise = 200, string iA_MC = "A_MC",
					    int iColor = -99, int iLineStyle = -99, 
					    int iMarkerStyle = -99, float iMarkerSize = -99. );
    bool         checkDataSetID( unsigned int iDataSetID );
    bool         fitResolution( TGraphErrors *g  );
    TGraphErrors* getLastPlottedGraph() { return gLastPlottedGraph; }
    unsigned int getNumberOfDataSets() { return fData.size(); }
    unsigned int getNumberOfGoodDataSets();
    TH1D*        getTheta2Histogram( unsigned int iDataSetID = 0, double i_Energy_TeV_lin = 1. );
    void         listDataSets();
    TCanvas*     plotAngularResolution( string iXaxis = "energy", string iProbabilityString = "68", double iMax = -1.e99 );
    TCanvas*     plotAngularResolution2D( unsigned int iDataSetID = 0, string iXaxis = "energy", string iProbabilityString = "68", double iEnergySlice_GeV = -1. );
    TCanvas*     plotCoreResolution( string iXaxis = "energy" );
    TCanvas*     plotCoreResolution2D( unsigned int iDataSetID = 0, string iXaxis = "energy" );
    void         plotCutEfficiency( unsigned int iDataSetID = 0 );
    void         plotCutEfficiencyRatio( unsigned int iDataSetID = 0, unsigned int iCutID = 999, double iPlotMaximum = 1.2 );
    TCanvas*     plotEffectiveArea( double iEffAreaMax_m2 = -1. );
    void         plotEffectiveAreaRatio( unsigned int iDataSetID = 0, double ymin = 0., double ymax = 1.3 );
    TCanvas*     plotEnergyReconstructionBias( string iM = "mean", double ymin = -1., double ymax = 1., bool iLogBias = false );
    void         plotEnergyReconstructionBias2D( unsigned int iDataSetID = 0, double iYmin = 0., double iYmax = 3. );
    void         plotEnergyReconstructionLogBias( string iM = "mean", double ymin = -1., double ymax = 1. );
    void         plotEnergyReconstructionLogBias2D( unsigned int iDataSetID = 0, string iM = "mean", double ymin = -0.8, double ymax = 0.8 );
    void         plotEnergyReconstructionMatrix( unsigned int iDataSetID = 0, bool bFineBinning = true, bool bQualityCuts = false, 
                                                 bool bInterPol = false, bool bPlotMedian = false );
    TCanvas*     plotEnergyResolution( double ymax = 1. );                                             // from geteffective area
    TCanvas*     plotEnergyResolution( string iXaxis );                                                // from IRF
    TCanvas*     plotEnergyResolution2D( unsigned int iDataSetID = 0, string iXaxis = "energy" );
    void         plotEnergySpectra( bool iWeighted = true, double iYMax = -1., int iRebin = 1 );
    void         plotTheta2( double iTheta2AxisMax = 0.05, bool iCumulative = false );
    void         plotTheta2( vector< double > i_Energy_TeV_lin, double iTheta2AxisMax = 0.05, bool iCumulative = false );
    bool         removeInstrumentResponseData( int iDataSetID );
    void         resetInstrumentResponseData();
    void         setCanvasSize( int iX = 600, int iY = 600 ) { fCanvasSize_X = iX; fCanvasSize_Y = iY; }
    void         setDebug( bool iB = true ) { fDebug = iB; }
    bool         setResolutionFitting( string iFitFunction = "", double iFitXmin = -1., double iFitXmax = 2. );
    void         setPlottingDefaults();
    void         setPlottingLogEnergyAxis( bool iB = true ) { if( getPlottingAxis( "energy_Lin" ) ) getPlottingAxis( "energy_Lin" )->fLogAxis = iB; }
    bool         write_fitResolutionFunction( string iOutName, string iName = "" );

};

#endif
