//! VEnergyCorrection get systematic error in energy reconstruction
// Revision $Id: VEnergyCorrection.h,v 1.1.2.1.4.1.12.1.18.1.4.1 2011/03/25 13:11:32 gmaier Exp $

#ifndef VENERGYCORRECTION_H
#define VENERGYCORRECTION_H

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

#include "VStatistics.h"
#include "VUtilities.h"

using namespace std;

class VEnergyCorrection
{
    private:

        bool fDebug;

        double degrad;
        double raddeg;

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
        double getTelescopeDistanceSC( unsigned int iTel1, unsigned int iTel2, double az, double z );
        double imageDistance( double c1x, double c2x, double c1y, double c2y );
        string testFileLocation( string iFile, string iDirectory );

    public:

        VEnergyCorrection();
        VEnergyCorrection( string iInputfile );
        ~VEnergyCorrection();
        void   calculateEmissionHeight( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el );
        double getMeanEmissionHeight() { return fEmissionHeight; }
        double getMeanEmissionHeightChi2() { return fEmissionHeightChi2; }
        vector< double >& getEmissionHeights() { return fEmissionHeightT; }
        unsigned int getNTelPairs() { return fNTelPairs; }
        double getEnergyCorrection( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el );
        double getEnergyCorrection( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el, bool iEcorr );
        double getEnergyCorrectionVariance() { return fEsysError; }
        double getEnergyCorrectionDistanceToEmissionMaximum() { return fEmissionHeight; }
        void setTelescopePositions( unsigned int ntel, double *x, double *y, double *z );
        void write();
};
#endif
