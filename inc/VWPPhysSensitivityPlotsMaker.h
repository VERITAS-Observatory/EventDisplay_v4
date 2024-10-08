//! VWPPhysSensitivityPlotsMaker class to produce a tex files for CTA/VTS sensitivities

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
        double fMinEnergy_TeV;
        double fMaxEnergy_TeV;

        string fPrintingOptions;

        int   fPlotCTARequirements;
        bool  fPlotCTARequirementGoals;

        // pads for plotAllInOneCanvas()
        TCanvas* fPlotAllInOneCanvas;
        TPad* fSensitivityPad;
        TPad* fSensitivityRatioPad;
        TPad* fEffAreaPad;
        TPad* fBckRatesPad;
        TPad* fERes;
        TPad* fAngRes;

    public:

        VWPPhysSensitivityPlotsMaker();
        ~VWPPhysSensitivityPlotsMaker() {}

        void compareDataSets( string iDataSetFile, string iDirectionString = "", bool IntegratedSensitivityForOffAxisPlots = false );
        void compareOffAxisSensitivities( string iSubArray, vector< string > iDataSet );
        void compareOffAxisSensitivities( string iSubArray = "", string iDataSet = "" );
        void plotAllInOneCanvas();
        void printPlotCTARequirementsIDs();
        void resetVectors();
        void setAxisUnits( double iMinSensitivity = 4.e-14, double iMaxSensitivity = 2.5e-10, string iUnit = "ENERGY" );
        void setEnergyRange_Lin_TeV( double iMinEnergy_TeV = 0.01, double iMaxEnergy_TeV = 200. )
        {
            fMinEnergy_TeV = iMinEnergy_TeV;
            fMaxEnergy_TeV = iMaxEnergy_TeV;
        }
        void setObservingTime( double i_s = 180000. )
        {
            fObservingTime_s = i_s;
        }
        void setPrintingOptions( string iPrint = "" )
        {
            fPrintingOptions = iPrint;
        }
        void setPlotRequirements( int iRequirementID = -1, bool iPlotRequirementGoals = false )
        {
            fPlotCTARequirements = iRequirementID;
            fPlotCTARequirementGoals = iPlotRequirementGoals;
        }
        void setOffAxisAngle( vector< double > iA )
        {
            fOffAxisAngle = iA;
        }
        bool writeTexFileBody( string iTexFile, string iTexFileTitle = "" );
};

#endif
