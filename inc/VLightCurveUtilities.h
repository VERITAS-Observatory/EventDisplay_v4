//! VLightCurveUtilities class handling basic light curve functions

#ifndef VLightCurveUtilities_H
#define VLightCurveUtilities_H

#include "VLightCurveData.h"

#include "TF1.h"
#include "TGraphAsymmErrors.h"
#include "TRandom.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VLightCurveUtilities
{
    private:

        bool fIsZombie;

        bool   fASCIIFormSecondColumnIsObservingInterval;

        bool   fXRTTimeSettings;
        double fXRTMissionTimeStart;

        double fLightCurveMJD_min;
        double fLightCurveMJD_max;

    protected:

        vector< VLightCurveData* >  fLightCurveData;

        // phases
        double   fPhase_MJD0;
        double   fPhase_Period_days;
        double   fPhaseError_low_fPhase_Period_days;
        double   fPhaseError_up_fPhase_Period_days;
        bool     fPhasePlotting;

    public:

        VLightCurveUtilities();
        ~VLightCurveUtilities() {}

        vector< VLightCurveData* > getLightCurveData()
        {
            return fLightCurveData;
        }
        double getLightCurveMJD_max()
        {
            return fLightCurveMJD_max;
        }
        double getLightCurveMJD_min()
        {
            return fLightCurveMJD_min;
        }
        double getFlux_Max();
        double getFlux_Min();
        double getFlux_Mean();
        double getFlux_Variance();
        double getFluxError_Mean();
        double getMeanObservationInterval();
        double getPhase( double iMJD );
        double getPhaseError( double iMJD );
        double getVariabilityIndex( TGraphAsymmErrors* g, double iSystematicFraction = 0. );
        bool   getXRTTimeSettings()
        {
            return fXRTTimeSettings;
        }
        bool   isZombie()
        {
            return fIsZombie;
        }
        void   printLightCurve( int bFullDetail = 1 );
        void   printLightCurveLaTexTableRow( double iSigmaMinFluxLimits = -99., double iFluxMultiplicator = 1., bool iPrintPhaseValues = false );
        void   printLightCurveDCF();
        void   printLightCurveWiki( double iMinEnergy_TeV = 1. );
        bool   readASCIIFile( string iFile, double iMJDMin = -99., double iMJDMax = -99., double iFluxMultiplier = 1. );
        void   resetLightCurveData();
        void   setASCIIFormSecondColumnIsObservingInterval( bool iB = true )
        {
            fASCIIFormSecondColumnIsObservingInterval = iB;
        }
        void   setPhaseFoldingValues( double iZeroPhase_MJD = -99., double iPhase_Days = 99.,
                                      double iPhaseError_low_Days = 0., double iPhaseError_up_Days = 0.,
                                      bool bPlotPhase = true );
        void   setXRTTimeSettings( bool iB = true, double iMJDMissionTimeStart = 54857.09977457897 )
        {
            fXRTTimeSettings = iB;
            fXRTMissionTimeStart = iMJDMissionTimeStart;             // MJD of time = 0
        }
        bool   updatePhaseFoldingValues();
        bool   writeASCIIFile( string iFile );
        bool   writeASCIIFile( string iFile, vector< VLightCurveData* > iV );
        bool   writeASCIIFile( string iFile, TF1* f, unsigned int iNPoints, double iMJD_min, double iMJD_max, double iFluxMeanError = 0.25, bool bClear = true );
};

#endif
