//! VEmissionHeightCalculator calculate emission height (and possibly get systematic error in energy reconstruction)

#ifndef VEmissionHeightCalculator_H
#define VEmissionHeightCalculator_H

#include "TFile.h"
#include "TF1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "VUtilities.h"

using namespace std;

class VEmissionHeightCalculator
{
    private:

        bool fDebug;

        double fEsys;
        double fEsysError;
        unsigned int fNTelPairs;
        double fEmissionHeight;
        double fEmissionHeightChi2;
        vector< double > fEmissionHeightT;

        TH1D *hHeight;
        TH2D *hHeightEsys;
        TProfile *hHeightEsysProf;

        unsigned int fNTel;
        vector< double > fTelX;
        vector< double > fTelY;
        vector< double > fTelZ;

        TFile *fInputFile;
        vector< string >  fZe;
        vector< double >  fZeDouble;
        vector< TF1* > fCorrectionCurves;
        vector< double > fCorrectionCurvesXmin;
        vector< double > fCorrectionCurvesXmax;

        double getEnergyCorrectionFromFunction( double iEmissionHeight, double iEl );
        double getEnergyCorrectionOrEmissionHeight( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el, bool iEcorr );
        double getTelescopeDistanceSC( unsigned int iTel1, unsigned int iTel2, double az, double z );
        double imageDistance( double c1x, double c2x, double c1y, double c2y );
        double interpolate( double w1, double ze1, double w2, double ze2, double ze, bool iCos = false,
               double iLimitforInterpolation = 0.5, double iMinValidValue = -90. );

    public:

        VEmissionHeightCalculator();
        VEmissionHeightCalculator( string iInputfile );
        ~VEmissionHeightCalculator();
        void   getEmissionHeight( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el );
        double getMeanEmissionHeight() { return fEmissionHeight; }
        double getMeanEmissionHeightChi2() { return fEmissionHeightChi2; }
        vector< double >& getEmissionHeights() { return fEmissionHeightT; }
        unsigned int getNTelPairs() { return fNTelPairs; }
        double getEnergyCorrection( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el );
        double getEnergyCorrectionVariance() { return fEsysError; }
        double getEnergyCorrectionDistanceToEmissionMaximum() { return fEmissionHeight; }
        void setTelescopePositions( unsigned int ntel, double *x, double *y, double *z );
        void write();
};
#endif
