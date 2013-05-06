//! VPlotWPPhysSensitivity plot CTA style sensitivities

#ifndef VPlotWPPhysSensitivity_H
#define VPlotWPPhysSensitivity_H

#include <fstream>
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

class VPlotWPPhysSensitivityData
{
   public:

   string fAnalysis;
   string fSensitivityFileName; 
   bool   fFileExists;
   double fObservationTime_s;
   string fSubArray;
   double fCameraOffset_deg;

   int    fPlottingColor;
   int    fPlottingLineStyle;
   int    fPlottingFillStyle;
   string fLegend;

   VPlotWPPhysSensitivityData();
  ~VPlotWPPhysSensitivityData() {}
   void   print();
};

//-----------------------------------------------------------------------

class VPlotWPPhysSensitivity : public VPlotUtilities
{
   private:

   VPlotInstrumentResponseFunction *fIRF;

   vector< VPlotWPPhysSensitivityData* > fData;

   double fMinEnergy_TeV;
   double fMaxEnergy_TeV;
   string fCrabSpectraFile;
   unsigned int fCrabSpectraID;

   bool    plotLegend( TCanvas *c = 0, bool iLeft = false );

   public:

   VPlotWPPhysSensitivity();
  ~VPlotWPPhysSensitivity() {}

   bool addDataSet( VPlotWPPhysSensitivityData* iData, bool iInit = true );
   bool addDataSet( string iAnalysis, string iSubArray = "E", double iObservationTime_s = 180000., double iOffset_deg = 0.0,
                    string iLegend = "", int iColor = 1, int iLineStyle = 1, int iFillStyle = 3001 );
   bool addDataSets( string iDataSettxtFile );
   vector< string > getListOfArrays();
   bool initialize( VPlotWPPhysSensitivityData* );
   bool plotIRF( string iPrint = "", double iEffAreaMin = 50., double iEffAreaMax = 5.e7, double iEnergyResolutionMax = 0.5 );
   bool plotSensitivity( string iPrint = "", double iMinSensitivity = 1.e-14, double iMaxSensitivity = 2.e-10, string iUnit = "ENERGY"  );
   void reset();
   void setCrabSpectraFile( string iFile = "$CTA_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat",
                            unsigned int iSpectraID = 6 )
                            { fCrabSpectraFile = iFile; fCrabSpectraID = iSpectraID; }
   void setEnergyRange_Lin_TeV( double iMinEnergy_TeV = 0.01, double iMaxEnergy_TeV = 200. ) 
                                { fMinEnergy_TeV = iMinEnergy_TeV; fMaxEnergy_TeV = iMaxEnergy_TeV; }

};

#endif
