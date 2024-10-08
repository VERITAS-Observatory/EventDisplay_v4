//! VTableCalculator calculation of mean and mean scaled variables

#ifndef VTableCalculator_H
#define VTableCalculator_H

#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include "TProfile2D.h"

#include "VGlobalRunParameter.h"
#include "VHistogramUtilities.h"
#include "VMedianCalculator.h"
#include "VStatistics.h"

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
        VTableCalculator( int intel = 0, bool iEnergy = false, bool iPE = false );
        VTableCalculator( string fpara, string hname, char m, TDirectory* iDir, bool iEnergy, bool iPE = false, int iUseMedianEnergy = 1 );

        // Destructor
        ~VTableCalculator() {}

        // Fill Histos and Calc Mean Scaled Width
        double calc( int ntel, double* r, double* s, double* w, double* mt, double& chi2, double& dE, double* st = 0 );
        const char* getInputTable()
        {
            if( fOutDir )
            {
                return fOutDir->GetName();
            }
            else
            {
                return "not defined";
            }
        }
        TH2F* getHistoMedian();
        TDirectory* getOutputDirectory()
        {
            return fOutDir;
        }
        void setCalculateEnergies()
        {
            fEnergy = true;
        }
        void setCalculateEnergies( bool iB )
        {
            fEnergy = iB;
        }
        void setDebug( unsigned int iB = 0 )
        {
            fDebug = iB;
        }
        void setMinRequiredShowerPerBin( float iM = 5. )
        {
            fMinShowerPerBin = iM;
        }
        void setVHistograms( vector< TH2F* >& hM );
        void setInterpolationConstants( int, int );
        void setOutputDirectory( TDirectory* iF )
        {
            fOutDir = iF;
        }
        void setWrite1DHistograms( bool iB )
        {
            fWrite1DHistograms = iB;
        }
        void terminate( TDirectory* iOut = 0, char* xtitle = 0 );

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
        string fHName_Add;

        bool fEnergy;                             //!< true if tables are used for energy calculation
        int  fUseMedianEnergy;

        bool fFillMedianApproximations;
        vector< vector< TH1F* > > Oh;
        vector< vector< VMedianCalculator* > > OMedian;
        TProfile2D* hMean;
        TH2F* hMedian;
        string hMedianName;
        vector< TH2F* > hVMedian;

        // histogram interpolation
        int fInterPolWidth;
        int fInterPolIter;

        TDirectory* fOutDir;
        bool fWrite1DHistograms;
        bool fReadHistogramsFromFile;

        char    Omode;
        bool    fwrite;

        bool   create1DHistogram( int i, int j, double w_first_event );
        bool   createMedianApprox( int i, int j );
        double getWeightMeanBinContent( TH2F*, int, int, double, double );
        void   fillMPV( TH2F*, int, int, TH1F*, double, double );
        double interpolate( TH2F* h, double x, double y, bool iError );
        bool   readHistograms();
        void   setBinning();
        void   setConstants( bool iPE = false );

};
#endif
