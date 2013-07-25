//!< VRadialAcceptance radial acceptance for a given point on the sky

#ifndef VACCEPTANCE_H
#define VACCEPTANCE_H

#include <cmath>
#include <iostream>
#include <string>

#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TH2F.h"
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
        double fMaxDistanceAllowed;
	double fCut_CameraFiducialSize_max;

	unsigned int fEnergyReconstructionMethod;

// regions excluded from background
        vector<double> fXE;
        vector<double> fYE;
        vector<double> fRE;

// calculate acceptance from data
        VGammaHadronCuts    *fCuts;
        VAnaSumRunParameter *fRunPar;

        TList *hList;
	TList *hListNormalizeHistograms;
	TList *hListFitHistograms;
        TH1F *hscale;
	TH1F *hPhiDist;
	TH1F *hPhiDistDeRot;
// acceptance curves as mean over all runs
        vector< double > fZe;                      //!< ze bins (upper limit of zenith angle bin)
	double fAzCut_min;
	double fAzCut_max;
        vector< TH1F* >  hAccZe;                   //!< zenith angle dependent acceptance curves
        vector< TF1* >   fAccZe;                   //!< zenith angle dependent acceptance curves

	vector< double > fPhiMin;                   //!< Phi bins (limits)
	vector< double > fPhiMax;                   //!< Phi bins (limits)
	vector< TH1F* >  hAccPhi;                   //!< azimuth angle dependent acceptance curves (azimuth angle in camera coordinates)
	vector< TH1F* >  hAccPhiDerot;              //!< azimuth angle dependent acceptance curves (azimuth angle in derotated camera coordinates)
        unsigned int     fAccZeFitMinBin;          //!< range (in bins) for normalisation of acceptance curves
        unsigned int     fAccZeFitMaxBin;

// acceptance vs yoff vs xoff
        TH2F *hXYAccTot;
        TH2F *hXYAccTotDeRot;
        vector< TH2F* > hXYAccRun;

// acceptance curves run wise, but mean over all zenith angles
        vector< TH1F* > hAccRun;                  //!< run dependent acceptance curves

// number of raw files used to calculate acceptance
        double fNumberOfRawFiles;

// get acceptance curves from a file
        TFile *fAccFile;

        void scaleArea( TH1F* );
        void reset();

    public:

        VRadialAcceptance();                                                            //!< use acceptance curve from simulation
        VRadialAcceptance( VGammaHadronCuts*iCuts, VAnaSumRunParameter *irun );         //!< set data source and cuts for acceptance curve calculation
        VRadialAcceptance( string ifile );                                              //!< use acceptance curve from this file
       ~VRadialAcceptance();

        int    fillAcceptanceFromData( CData *c, int entry );
        double getAcceptance( double x, double y, double erec = 0., double ze = 0. );   //!< return radial acceptance
        double getCorrectionFactor( double x, double y, double erec );                  //!< return correction factor (1/radial acceptance)
        double getNumberofRawFiles() { return fNumberOfRawFiles; }
        bool   isExcluded( double, double );                                            //!< region excluded from analysis
        bool   isExcludedfromBackground( double, double );                              //!< region excluded from background analysis
        bool   isExcludedfromSource( double, double );                                  //!< region excluded from source analyis
	void   setAzCut( double iAzMin = -1.e9, double iAzMax = 1.e9 )  { fAzCut_min = iAzMin; fAzCut_max = iAzMax; } //!< cut on Az (shower directory)
	void   setEnergyReconstructionMethod( unsigned int iEMethod = 0 ) { fEnergyReconstructionMethod = iEMethod; }
        void   setSource( double x, double y, double r, double idist, double imaxdist = 5. ); //!< set source position, radius, and minimal distance between source and background
        void   setRegionToExcludeAcceptance( vector<double> x, vector<double> y, vector<double> r ); //set the region to be exclude in the analysis
        bool   terminate( TDirectory *iDirectory );
};
#endif
