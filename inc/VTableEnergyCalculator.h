//! VTableEnergyCalculator fill or lookup energy tables
#ifndef VTableEnergyCalculator_H
#define VTableEnergyCalculator_H

#include "TDirectory.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include "TProfile2D.h"

#include "VGlobalRunParameter.h"
#include "VInterpolate2DHistos.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


class VTableEnergyCalculator
{
    public:
// creator: reads or writes table
// mode can be 'r' or 'w'
        VTableEnergyCalculator( int intel = 0 );
        VTableEnergyCalculator( const char* fname, char m, TDirectory *iDir, bool iUseMedian = true, string iInterpolate = "" );

// Destructor
        ~VTableEnergyCalculator() {}

// Fill Histos and Compute Energy
        double calc(int ntel, double e, double *r, double *s,double *dist,double *et,double &chi2, double &dE, double eys );
        TH2F* getHistoMedian();
        TH2F* getHistoSigma();
        const char* getInputTable() { if( fOutDir ) return fOutDir->GetName(); else return "not defined"; }
        TDirectory* getOutputDirectory() { return fOutDir; }
        double getMinSizePerTelescope() { return fMinSize; }
        double getMaxDistanceFromTelescope() { return fMaxDistance; }
        double getMaxLocalDistanceFromTelescope() { return fMaxLocalDistance; }
        void setVHistograms( vector< TH2F* >& hM );
        void setMinSizePerTelescope( double iS ) { fMinSize = iS; }
        void setCutValues( double iSize, double iLocalDist, double iDist );
	void setMinRequiredShowerPerBin( float iM = 5. ) { fMinShowerPerBin = iM; }
        void setInterpolationConstants( int, int );
        void setOutputDirectory( TDirectory *iDir ) { fOutDir = iDir; }
        void setWrite1DHistograms( bool iB ) { fWrite1DHistograms = iB; }
        void terminate( TDirectory *iDir = 0, char *xtitle = 0 );

    private:

        int fDebug;

        bool  fUseMedianEnergy;
	float fMinShowerPerBin;        // minimum number per bin required (table writing)

// BINNING DEFINITION FOR LOOKUP TABLES
        int   eNumEne;               // number of bins on energy axis
	float eE0Min;                // minimum energy linE [TeV] (must be >0)
	float eE0Max;                // maximum energy linE [TeV] (must be >0)
	int   eHistBins;             // number of bins on size axis (1D histos!)
	float exlow;                 // minimum log10 size
	float exhigh;                // maximum log10 size
        int   eNumDist;              // number of bins on core distance axis
	float edist_delta;           // bin width for core distance axis [m]

        vector< vector< TH1F* > > Oh;
        TH2F *hMedian;
        string hMedianName;
        TH2F *hSigma;
        string hSigmaName;
        TH2F *hNevents;
        string hNeventsName;
        vector< TH2F* > hVMedian;
        TProfile2D *hMean;
	string hMeanName;
        TDirectory *fOutDir;
        bool fWrite1DHistograms;
        bool fReadHistogramsFromFile;

	string fHName_add;

        string fInterpolationString;

        double fMinSize;                          //!< minimum size per telescope to be included in the analysis
        double fMaxDistance;                      //!< maximum distance of shower core from telescope [m]
        double fMaxLocalDistance;                 //!< maximum local distance of image from camera center [deg]

// energy correction stuff
        vector< double > sstop;
        vector< double > fstart;
        vector< double > fstop;
        vector< double > p0;
        vector< double > p1;

// histogram interpolation
        int fInterPolWidth;
        int fInterPolIter;

        char    Omode;

	bool create1DHistogram( int i, int j );
        void get_logEnergy(double logSize, int ir, double &med, double &sigma, unsigned int itel = 0 );
        void get_logEnergy2D(double logSize, double r, double &med, double &sigma, unsigned int itel = 0 );
        void initialize();
        bool readHistograms();
	void setConstants();
};
#endif
