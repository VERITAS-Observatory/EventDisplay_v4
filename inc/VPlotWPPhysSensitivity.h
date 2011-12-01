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
   vector< string > fSubArray;
   vector< double > fCameraOffset_deg;

   vector< string > fSensitivityFile;   // [Analysis][SubArray+SObsTime];
   vector< double > fIRFCameraOffset_deg;
   vector< int >    fPlottingColor;
   vector< int >    fPlottingLineStyle;

   public:

   VPlotWPPhysSensitivity();
  ~VPlotWPPhysSensitivity() {}

   void addAnalysis( string iAnalysis, int iColor = 1, int iLineStyle = -1 );           // NOTE: hardwired analysis file names
   void addCameraOffset( double iCameraOffset_deg = 0. );
   void addObservationTime( double iObsTime );
   void addSubArray( string iArray );
   bool initialize();
   bool plotIRF();
   bool plotSensitivity();

};

#endif
