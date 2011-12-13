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

class VPlotWPPhysSensitivity : public VPlotUtilities
{
   private:

   VPlotInstrumentResponseFunction *fIRF;

   vector< string > fAnalysis;
   vector< int >    fAnalysisColor;
   vector< int >    fAnalysisLineStyle;
   vector< double > fObservationTime_H;
   vector< int >    fObservationTimeColor;
   vector< int >    fObservationTimeLineStyle;
   vector< string > fSubArray;
   vector< int >    fSubArrayColor;
   vector< int >    fSubArrayLineStyle;
   vector< double > fCameraOffset_deg;
   vector< int >    fCameraOffsetColor;
   vector< int >    fCameraOffsetLineStyle;

   vector< string > fSensitivityFile;   // [Analysis][SubArray+SObsTime];
   vector< double > fIRFCameraOffset_deg;
   vector< int >    fPlottingColor;
   vector< int >    fPlottingLineStyle;
   vector< string > fLegend;

   bool    plotLegend( TCanvas *c = 0, bool iLeft = false );

   public:

   VPlotWPPhysSensitivity();
  ~VPlotWPPhysSensitivity() {}

   void addAnalysis( string iAnalysis, int iColor = -1, int iLineStyle = -1 );           // NOTE: hardwired analysis file names
   void addCameraOffset( double iCameraOffset_deg = 0., int iColor = -1, int iLineStyle = -1 );
   void addObservationTime( double iObsTime, int iColor = -1, int iLineStyle = -1 );
   void addSubArray( string iArray, int iColor = -1, int iLineStyle = -1 );
   bool initialize();
   bool plotIRF( string iPrint = "" );
   bool plotSensitivity( string iPrint = "" );

};

#endif
