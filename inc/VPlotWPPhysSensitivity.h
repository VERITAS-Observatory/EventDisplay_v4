//! VPlotWPPhysSensitivity

#ifndef VPlotWPPhysSensitivity_H
#define VPlotWPPhysSensitivity_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TLegend.h"

#include "VPlotInstrumentResponseFunction.h"
#include "VPlotUtilities.h"
#include "VSensitivityCalculator.h"

class VWPPhysMinimumRequirements
{
    public:

    string fName;
    double fEnergyRange_TeV_Min;
    double fEnergyRange_TeV_Max;
    double fEnergyThreshold_TeV;
    vector< double > fEnergyResolution_Energy_TeV_Min;
    vector< double > fEnergyResolution_Energy_TeV_Max;
    vector< double > fEnergyResolution;
    vector< double > fAngularResolution_Energy_TeV_Min;
    vector< double > fAngularResolution_Energy_TeV_Max;
    vector< double > fAngularResolution_deg;
    vector< double > fDifferentalSensitivity_Energy_TeV;
    vector< double > fDifferentalSensitivity_erg_s_m2;

    VWPPhysMinimumRequirements( string iName );
   ~VWPPhysMinimumRequirements() {}
    bool readWPPhysMinimumRequirements( string iFile );
};

class VPlotWPPhysSensitivity : public VPlotUtilities
{
   private:

   VPlotInstrumentResponseFunction *fIRF;

   vector< string > fAnalysis;
   vector< int >    fAnalysisColor;
   vector< int >    fAnalysisLineStyle;
   vector< int >    fAnalysisFillStyle;

   vector< double > fObservationTime_H;
   vector< int >    fObservationTimeColor;
   vector< int >    fObservationTimeLineStyle;
   vector< int >    fObservationTimeFillStyle;

   vector< string > fSubArray;
   vector< int >    fSubArrayColor;
   vector< int >    fSubArrayLineStyle;
   vector< int >    fSubArrayFillStyle;

   vector< double > fCameraOffset_deg;
   vector< int >    fCameraOffsetColor;
   vector< int >    fCameraOffsetLineStyle;
   vector< int >    fCameraOffsetFillStyle;

   vector< string > fSensitivityFile;   // 
   vector< double > fIRFCameraOffset_deg;
   vector< int >    fPlottingColor;
   vector< int >    fPlottingLineStyle;
   vector< int >    fPlottingFillStyle;
   vector< string > fLegend;

   double fMinEnergy_TeV;
   double fMaxEnergy_TeV;

   bool    plotLegend( TCanvas *c = 0, bool iLeft = false );

   public:

   VPlotWPPhysSensitivity();
  ~VPlotWPPhysSensitivity() {}

   void addAnalysis( string iAnalysis, int iColor = -1, int iLineStyle = -1, int iFillStyle = -1 );           // NOTE: hardwired analysis file names
   void addCameraOffset( double iCameraOffset_deg = 0., int iColor = -1, int iLineStyle = -1, int iFillStyle = -1 );
   void addObservationTime( double iObsTime, int iColor = -1, int iLineStyle = -1, int iFillStyle = -1 );
   void addSensitivityFile( string iSensitivityFile, string iLegend, int iColor = -1, int iLineStyle = -1, int iFillStyle = -1 );
   void addSubArray( string iArray, int iColor = -1, int iLineStyle = -1, int iFillStyle = -1 );
   bool initialize();
   bool plotIRF( string iPrint = "", double iEffAreaMax = 5.e7, double iEnergyResolutionMax = 0.5 );
   bool plotSensitivity( string iPrint = "", double iMinSensitivity = 1.e-14, double iMaxSensitivity = 2.e-10, string iUnit = "ENERGY"  );
   void setEnergyRange_Lin_TeV( double iMinEnergy_TeV = 0.01, double iMaxEnergy_TeV = 50. ) { fMinEnergy_TeV = iMinEnergy_TeV; fMaxEnergy_TeV = iMaxEnergy_TeV; }

};

#endif
