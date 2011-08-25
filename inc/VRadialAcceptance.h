//!< VRadialAcceptance radial acceptance for a given point on the sky
//  Revision $Id: VRadialAcceptance.h,v 1.9.2.2.4.2.12.1.4.1.2.1.2.1.10.1 2010/03/08 07:45:08 gmaier Exp $

#ifndef VACCEPTANCE_H
#define VACCEPTANCE_H

#include "CData.h"

#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TKey.h"
#include "TList.h"

#include "VGammaHadronCuts.h"
#include "VAstroSource.h"
#include "VAnaSumRunParameter.h"
#include "VSkyCoordinatesUtilities.h"
#include "VTargets.h"
#include "VUtilities.h"

#include "cmath"

using namespace std;

Double_t VRadialAcceptance_fit_acceptance_function(Double_t *x, Double_t *par);

class VRadialAcceptance
{
    private:

        double degrad;

        bool fSimuAcceptance;

        bool fAcceptanceFunctionDefined;

        double fXs;
        double fYs;
        double fRs;
        double fDs;
        double fMs;

// regions excluded from background
        vector<double> fXE;
        vector<double> fYE;
        vector<double> fRE;

// acceptance curves from simulations

        TF1 *fAccp5;
        TH1D *hAccp5;

        void initAcceptancefromSimulations();

// calculate acceptance from data
        VGammaHadronCuts *fCuts;
        VAnaSumRunParameter *fRunPar;
        VAstroSource *fAstro;

        TList *hList;
        TH1D *hscale;
// acceptance curves as mean over all runs
        vector< double > fZe;                     //!< ze bins (upper limit of zenith angle bin)
        vector< TH1D* > hAccZe;                   //!< zenith angle dependent acceptance curves
        vector< TF1* > fAccZe;                    //!< zenith angle dependent acceptance curves
        vector< TH1D* > hAccZeFit;                //!< zenith angle dependent acceptance curves
        unsigned int fAccZeFitMinBin;             //!< range (in bins) for normalisation of acceptance curves
        unsigned int fAccZeFitMaxBin;

// azimuth angle dependent acceptance curves, mean over all zenith angles
        vector< double > fAz;
        vector< TH1D* > hAccAz;
// acceptance vs yoff vs xoff, azimuth angle dependent
        TH2D *hXYAccTot;
        vector< TH2D* > hXYAcc;

// acceptance curves run wise, but mean over all zenith angles
        vector< TH1D* > hAccRun;                  //!< run dependent acceptance curves

// number of raw files used to calculate acceptance
        double fNumberOfRawFiles;

// get acceptance curves from a file
        TFile *fAccFile;

        void scaleArea( TH1D* );

        void reset();

    public:

        VRadialAcceptance();                            //!< use acceptance curve from simulation
                                                  //!< set data source and cuts for acceptance curve calculation
        VRadialAcceptance( VGammaHadronCuts*iCuts, VAnaSumRunParameter *irun );
        VRadialAcceptance( string ifile );              //!< use acceptance curve from this file
        ~VRadialAcceptance();

        bool fillAcceptanceFromData( CData *c );
                                                  //!< acceptance
        double getAcceptance( double x, double y, double erec = 0., double ze = 0. );
                                                  //!< correction factor (1/acceptance)
        double getCorrectionFactor( double x, double y, double erec );
        TF1 *getFunction()                        //!< return acceptance function
        {
            return fAccp5;
        }
        TH1D* getAcceptanceHistogram()            //!< return acceptance histogram
        {
            return hAccp5;
        }
        double getNumberofRawFiles();
        bool isExcluded( double, double );        //!< region excluded from analysis
                                                  //!< region excluded from background analysis
        bool isExcludedfromBackground( double, double );
                                                  //!< region excluded from source analyis
        bool isExcludedfromSource( double, double );
                                                  //!< set source position, radius, and minimal distance between source and background
        void setSource( double x, double y, double r, double idist );
                                                  //!< set source position, radius, and minimal distance between source and background
        void setSource( double x, double y, double r, double idist, double imaxdist );
                                                  //set the region to be exclude in the analysis
        void setRegionToExcludeAcceptance( vector<double> x, vector<double> y, vector<double> r );
        bool terminate( string ofile );
};
#endif
