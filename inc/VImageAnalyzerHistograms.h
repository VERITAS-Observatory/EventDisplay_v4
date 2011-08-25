//! VImageAnalyzerHistograms  histogramming of run parameter, etc.

#ifndef VImageAnalyzerHistograms_H
#define VImageAnalyzerHistograms_H

#include <TFile.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TList.h>
#include <TTree.h>

#include <iostream>
#include <string>
#include <valarray>
#include <vector>

using namespace std;

class VImageAnalyzerHistograms
{
    private:
        unsigned int fTelescopeID;
        unsigned int fNChannels;
        double fRunLength;

    public:
        VImageAnalyzerHistograms( unsigned int iTel );
        ~VImageAnalyzerHistograms();
        void init();                              //!< book histograms
        void fill( vector< bool >& iImage, vector<bool>& iBorder, valarray<double>& iSums, vector<bool>& iHiLo, double itstart );
        void fillDeadChannelTree( vector< unsigned int >& iDead, vector< unsigned int >& iDeadLow );
        void fillDiagnosticTree( int rN, int eN, int iMJD, double it, vector< double >& iF, vector< double >& iS );
        void setNChannels( unsigned int iChannels ) { fNChannels = iChannels; }
        void setRunLength( double iL ) { fRunLength = iL; }
        void terminate( TFile* );                 //!< write results to same file as VAnalyzer class

        TList *hisList;
        TH2D *fHis_image;
        TH2D *fHis_border;
        TH2D *fHis_hilo;                          //!< high/low gain distribution
        TH1F *fHis_sums;

        TTree *fdiagno;
        int runNumber;
        int eventNumber;
        int MJD;
        double time;
        double fFADCstopTZero[4];
        double fFADCstopSum[4];

        TTree *fDeadChannels;
        int channel;
        int dead;
        int deadLow;
};
#endif                                            // VImageAnalyzerHistograms_H
