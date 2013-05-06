//! VPlotTMVAParameters plot results from TMVA optimization

#ifndef VPlotTMVAParameters_H
#define VPlotTMVAParameters_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TH1D.h"

#include "VTMVAEvaluator.h"

using namespace std;

class VPlotTMVAParameters
{
   private:

    vector< string > fSubArrays;
    string  fDataDirectory;

    vector< TH1D* > hSignalEfficiency;
    vector< TH1D* > hBackgroundEfficiency;

    bool initializeHistograms( unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max );

   public:
    VPlotTMVAParameters();
   ~VPlotTMVAParameters() {}

    void initializeWeightFiles( string iDirectory, string iTMVADirectory,
                                unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max );
    void plot();
    void setDirectories( string iDataDirectory ) { fDataDirectory = iDataDirectory; }
    bool setSubArrays( string iSubarrayFile );
};



#endif
