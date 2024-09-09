//! VEnergyThreshold  class to determine energy threshold

#ifndef VEnergyThreshold_H
#define VEnergyThreshold_H

#include "TCanvas.h"
#include "TChain.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TH2F.h"
#include "TList.h"
#include "TMath.h"
#include "TObject.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TText.h"
#include "TTree.h"

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "VRunList.h"

using namespace std;

class VEnergyThreshold : public TObject
{
    private:

        bool fDebug;

        TFile* fEffAreaFile;
        TChain* fEffArea;

        TFile* fEnergyThresholdFile;

        double fEnergyThresholdFixedValue;
        string fEnergyThresholdFileName;

        TFile* fOutFile;
        TTree* fTreeEth;

        float fze;
        UShort_t fAzBin;
        float fAzMin;
        float fAzMax;
        float fWoff;
        UShort_t fTNoise;
        float fTPedvar;
        float feth;
        float fesys_10p;
        float fesys_15p;
        float fesys_20p;
        float feffFract_05p;
        float feffFract_10p;
        float feffFract_20p;
        float feffFract_50p;
        float feffFract_90p;
        float feff_150GeV;
        float feff_300GeV;
        float feff_500GeV;
        float feff_1TeV;
        float feff_10TeV;

        UShort_t nbins;
        float e0[1000];
        float eff[1000];
        UShort_t nbins_esys;
        float e0_esys[1000];
        float esys_rel[1000];

        int fPlottingMarkerStyle;
        int fPlottingMarkerColor;
        float fPlottingMarkerSize;
        float fPlottingLineWidth;
        double fPlottingYmin;
        double fPlottingYmax;

        double getEnergyThreshold( TH1D* h = 0, bool bLogEnergyAxis = true, bool bFit = true );
        bool openEnergyThresholdFile();
        bool setUpThresholdTree();

        double interpolateEnergyThreshold( VRunList* );

    public:

        VEnergyThreshold();
        VEnergyThreshold( string ioutfilename );
        VEnergyThreshold( double iEthFixed, string iInFile = "" );
        ~VEnergyThreshold();
        bool closeOutputFile();
        bool openEffectiveAreaFile( string ifile );
        bool calculateEnergyThreshold( bool bFit = true, int nentries = -1 );
        double getEnergy_maxSystematic( vector< double > x, vector< double > y, double iSys = 0.1 );
        double getEnergy_maxSystematic( TGraphErrors* g, double iSys = 0.1 );
        double getEnergy_MaxEffectiveAreaFraction( TObject* h = 0, double iFrac = 0.1 );
        double getEnergy_fixedValue()
        {
            return fEnergyThresholdFixedValue;
        }
        void plot_energyThresholds( string var = "E_diffmax", double ze = 20., double woff = 0.5, int noise = 150, int az = 16, bool bPlot = true, string plot_option = "p" );
        void setPlottingStyle( int iC = 1, int iS = 21, float iW = 2., float iL = 2. )
        {
            fPlottingMarkerStyle = iS;
            fPlottingMarkerColor = iC;
            fPlottingMarkerSize = iW;
            fPlottingLineWidth = ( Width_t )iL;
        }
        void setPlottingYaxis( double ymin = -1., double ymax = -1. )
        {
            fPlottingYmin = ymin;
            fPlottingYmax = ymax;
        }
        bool writeResults();

        double getEnergyThreshold( VRunList* );

        ClassDef( VEnergyThreshold, 2 );
};
#endif
