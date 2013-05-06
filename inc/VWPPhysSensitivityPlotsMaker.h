//! VWPPhysSensitivityPlotsMaker class to produce a tex files for CTA/VTS sensitivties

#ifndef VWPPhysSensitivityPlotsMaker_H
#define VWPPhysSensitivityPlotsMaker_H

#include "VPlotWPPhysSensitivity.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VWPPhysSensitivityPlotsMaker
{
    private:

    double           fObservingTime_s;
    vector< string > fListofDataSets;
    vector< string > fListOfArrays;
    vector< double > fOffAxisAngle;

    double fSensitivity_min;
    double fSensitivity_max;
    string fSensitivity_Unit;

    string fPrintingOptions;

    bool  getListOfDataSets( string iiDataSetFileTxt );
    bool  getListOfArrays( string iSubArrayFileTxt );
    bool  readList( string iFileTxt, vector< string >& iList );

    public:

    VWPPhysSensitivityPlotsMaker();
   ~VWPPhysSensitivityPlotsMaker() {}

    void compareDataSets( string iDataSetFile );
    void compareDataSets( string iSubArray, string iDataSet );
    void compareOffAxisSensitivities( string iSubArray = "", string iDataSet = "" );
    void resetVectors();
    void setAxisUnits( double iMinSensitivity = 1.e-14, double iMaxSensitivity = 2.e-10, string iUnit = "ENERGY"  );
    bool setListOfArrays( string iSubArrayFileTxt );
    bool setListOfDataSets( string iDataSetFileTxt );
    void setObservingTime( double i_s = 180000. ) { fObservingTime_s = i_s; }
    void setPrintingOptions( string iPrint = "" ) { fPrintingOptions = iPrint; }
    void setOffAxisAngle( vector< double > iA ) { fOffAxisAngle = iA; }
    bool writeTexFileBody( string iTexFile, string iTexFileTitle = "" );
};

#endif
