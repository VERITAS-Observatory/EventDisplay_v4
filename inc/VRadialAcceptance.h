//!< VRadialAcceptance radial acceptance for a given point on the sky
//  Revision $Id: VRadialAcceptance.h,v 1.9.2.2.4.2.12.1.4.1.2.1.2.1.10.1 2010/03/08 07:45:08 gmaier Exp $

#ifndef VACCEPTANCE_H
#define VACCEPTANCE_H

#include <cmath>
#include <iostream>
#include <string>

#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TKey.h"
#include "TList.h"

#include "CData.h"
#include "VGammaHadronCuts.h"
#include "VAnaSumRunParameter.h"
#include "VUtilities.h"

using namespace std;

// fit function for radial acceptance
Double_t VRadialAcceptance_fit_acceptance_function(Double_t *x, Double_t *par);

// class definition
class VRadialAcceptance
{
    private:

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

// calculate acceptance from data
        VGammaHadronCuts    *fCuts;
        VAnaSumRunParameter *fRunPar;

        TList *hList;
	TList *hListNormalizeHistograms;
        TH1D *hscale;
// acceptance curves as mean over all runs
        vector< double > fZe;                      //!< ze bins (upper limit of zenith angle bin)
        vector< TH1D* >  hAccZe;                   //!< zenith angle dependent acceptance curves
        vector< TF1* >   fAccZe;                   //!< zenith angle dependent acceptance curves
        vector< TH1D* >  hAccZeFit;                //!< zenith angle dependent acceptance curves
	vector< double > fAzMin;                   //!< az bins (limits)
	vector< double > fAzMax;                   //!< az bins (limits)
	vector< TH1D* >  hAccAz;                   //!< azimuth angle dependent acceptance curves
        unsigned int     fAccZeFitMinBin;          //!< range (in bins) for normalisation of acceptance curves
        unsigned int     fAccZeFitMaxBin;

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

        VRadialAcceptance();                                                            //!< use acceptance curve from simulation
        VRadialAcceptance( VGammaHadronCuts*iCuts, VAnaSumRunParameter *irun );         //!< set data source and cuts for acceptance curve calculation
        VRadialAcceptance( string ifile );                                              //!< use acceptance curve from this file
       ~VRadialAcceptance();

        bool   fillAcceptanceFromData( CData *c );
        double getAcceptance( double x, double y, double erec = 0., double ze = 0. );   //!< return radial acceptance
        double getCorrectionFactor( double x, double y, double erec );                  //!< return correction factor (1/radial acceptance)
        double getNumberofRawFiles() { return fNumberOfRawFiles; }
        bool   isExcluded( double, double );                                            //!< region excluded from analysis
        bool   isExcludedfromBackground( double, double );                              //!< region excluded from background analysis
        bool   isExcludedfromSource( double, double );                                  //!< region excluded from source analyis
        void   setSource( double x, double y, double r, double idist, double imaxdist = 5. ); //!< set source position, radius, and minimal distance between source and background
        void   setRegionToExcludeAcceptance( vector<double> x, vector<double> y, vector<double> r ); //set the region to be exclude in the analysis
        bool   terminate( string ofile );
};
#endif
