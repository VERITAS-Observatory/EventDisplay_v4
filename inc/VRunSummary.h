//! VRunSummary  class for rate/significance results from each run
//  Revision $Id: VRunSummary.h,v 1.9.2.1.18.1.4.1.4.2.8.1.2.1 2010/03/08 07:45:10 gmaier Exp $

#ifndef VRUNSUMMARY_H
#define VRUNSUMMARY_H

#include "CRunSummary.h"
#include "VAnaSumRunParameter.h"

#include <iomanip>
#include <iostream>
#include <string>

#include "TF1.h"
#include "TGraphAsymmErrors.h"
#include "TH1D.h"
#include "TMinuit.h"
#include "TTree.h"

using namespace std;

class VRunSummary
{
    private:
        TTree *fRunSummaryTree;

        void fitEnergyHistogram( TH1D*, double minE, double maxE );
        void init();
        bool initTree();
        bool setBranches();

    public:

        int runOn;
        int runOff;
        double MJDOn;
        double MJDOff;
        double fTargetRA;
        double fTargetDec;
        double fTargetRAJ2000;
        double fTargetDecJ2000;
        double fSkyMapCentreRAJ2000;
        double fSkyMapCentreDecJ2000;
        double fTargetShiftRAJ2000;
        double fTargetShiftDecJ2000;
        double fTargetShiftNorth;
        double fTargetShiftWest;
        double fWobbleNorth;
        double fWobbleWest;
        unsigned int fNTel;
        double tOn;
        double tOff;
        double elevationOn;
        double azimuthOn;
        double elevationOff;
        double azimuthOff;
        double RawRateOn;
        double RawRateOff;
        double pedvarsOn;
        double pedvarsOff;
        double NOn;
        double NOff;
        double NOffNorm;
        double OffNorm;
        double Signi;
        double Rate;
        double RateE;
        double RateOff;
        double RateOffE;
        double DeadTimeFracOn;
        double DeadTimeFracOff;
        double MaxSigni;
        double MaxSigniX;
        double MaxSigniY;

////////////////
// variables used for total analysis only
//
        map< int, double >  fRunMJD;
        double fTotalExposureOn;
        double fTotalExposureOff;
        map< int, double > f_exposureOn;
        map< int, double > f_exposureOff;
        double fMeanElevationOn;
        double fMeanElevationOff;
        double fMeanAzimuthOn;
        double fMeanAzimuthOff;
        double fNMeanElevation;
        double fMeanDeadTimeOn;
        double fMeanDeadTimeOff;
        double fMeanRawRateOn;
        double fMeanRawRateOff;
        double fMeanPedVarsOn;
        double fMeanPedVarsOff;

        double fTotTargetRA;
        double fTotTargetDec;
        double fTotTargetRAJ2000;
        double fTotTargetDecJ2000;

        VRunSummary();
        ~VRunSummary() {}
        void fill();
        bool fill( string, string, vector< sRunPara > );
        void print();
        void write();
};
#endif
