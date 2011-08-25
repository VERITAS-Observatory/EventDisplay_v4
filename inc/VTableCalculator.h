//! VTableCalculator calculation of mean and mean scaled variables
// Revision $Id$

#ifndef VTableCalculator_H
#define VTableCalculator_H

#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile2D.h"

#include "VGlobalRunParameter.h"
#include "VInterpolate2DHistos.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VTableCalculator
{
    public:
// creator: reads or writes table
// mode can be 'r' or 'w'
        VTableCalculator( int intel = 0 , bool iEnergy = false, bool iPE = false );
        VTableCalculator( vector< TH2F* > iMedian, vector< TH2F* > iSigma, bool iEnergy, bool iPE = false );
        VTableCalculator( string fpara, string hname, char m, TDirectory *iDir, bool iEnergy, string iInterpolate = "", bool iPE = false );

// Destructor
        ~VTableCalculator() {}

// Fill Histos and Calc Mean Scaled Width
        double calc(int ntel, double *r, double *s, double *w, double *mt, double &chi2, double &dE, double *st = 0 );
        const char* getInputTable() { if( fOutDir ) return fOutDir->GetName(); else return "not defined"; }
        TH2F* getHistoMedian();
        TH2F* getHistoSigma();
        TDirectory *getOutputDirectory() { return fOutDir; }
        void setCalculateEnergies() { fEnergy = true; }
        void setCalculateEnergies( bool iB ) { fEnergy = iB; }
	void setDebug( unsigned int iB = 0 ) { fDebug = iB; }
	void setMinRequiredShowerPerBin( float iM = 5. ) { fMinShowerPerBin = iM; }
        void setVHistograms( vector< TH2F* >& hM, vector< TH2F* >& hS );
        void setInterpolationConstants( int, int );
        void setOutputDirectory( TDirectory *iF ) { fOutDir = iF; }
        void setWrite1DHistograms( bool iB ) { fWrite1DHistograms = iB; }
        void terminate( TDirectory *iOut = 0, char *xtitle = 0 );

    private:
        unsigned int fDebug;

        double fBinning1DXlow;
        double fBinning1DXhigh;

	float fMinShowerPerBin;        // minimum number per bin required (table writing)

// histogram definitions
        int   NumSize;
        float amp_offset;
        float amp_delta;
        int   NumDist;
        float dist_delta;
        int   HistBins;
        float xlow;
        float xhigh;

        string fName;

        bool fEnergy;                             //!< true if tables are used for energy calculation

        vector< vector< TH1F* > > Oh;
        TProfile2D *hMean;
        TH2F* hMedian;
        string hMedianName;
        TH2F* hSigma;
        string hSigmaName;
        TH2F* hNevents;
        vector< TH2F* > hVMedian;
        vector< TH2F* > hVSigma;

// histogram interpolation
        int fInterPolWidth;
        int fInterPolIter;

        TDirectory *fOutDir;
        bool fWrite1DHistograms;
        bool fReadHistogramsFromFile;

        char    Omode;

        int SizeIndex(double size);
        int DistIndex(double dist);
        double getWeightMeanBinContent( TH2F*, int, int, double, double );
        bool   readHistograms();
        void   setConstants( bool iPE = false );
};
#endif
