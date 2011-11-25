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
   vector< double > fObservationTime_H;
   vector< string > fSubArray;

   vector< string > fSensitivityFile;   // [Analysis][SubArray+SObsTime];
   vector< int >    fPlottingColor;
   vector< int >    fPlottingLineStyle;

   public:

   VPlotWPPhysSensitivity();
  ~VPlotWPPhysSensitivity() {}

   void addAnalysis( string iAnalysis, int iColor = 1 );           // NOTE: hardwired analysis file names
   void addObservationTime( double iObsTime );
   void addSubArray( string iArray );
   bool initialize();
   bool plotIRF();
   bool plotSensitivity();

};

#endif
