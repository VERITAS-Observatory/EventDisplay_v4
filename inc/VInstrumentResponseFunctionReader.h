//! VInstrumentResponseFunctionReader data class for reading response functions (effective area, angular resolution, etc )

#ifndef VInstrumentResponseFunctionReader_H
#define VInstrumentResponseFunctionReader_H

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TH2F.h"
#include "TMath.h"
#include "TProfile.h"
#include "TTree.h"

#include "CEffArea.h"
#include "VAnalysisUtilities.h"
#include "VGammaHadronCuts.h"
#include "VHistogramUtilities.h"
#include "VInstrumentResponseFunctionData.h"
#include "VPlotUtilities.h"

using namespace std;

class VInstrumentResponseFunctionReader : public VAnalysisUtilities, public VPlotUtilities, public VHistogramUtilities
{
    private:
    
        bool   fIsZombie;
        bool   fDebug;
        
        int    fGammaHadronCuts_directionCut_selector;
        
        bool                             calculateCutEfficiencies();
        TGraphAsymmErrors*               calculateEffectiveAreaRatios( TGraphAsymmErrors* g0, TGraphAsymmErrors* g1 );
        bool                             fill_from_effectiveArea( TTree* t );
        bool                             fill_from_effectiveAreaFromH2( TTree* t );
        VInstrumentResponseFunctionData* getIRFFromFile( TTree*, unsigned int );
        bool                             getDataFromFile();
        bool                             getDataFromCTAFile();
        void                             getEnergyResolutionPlot( TProfile* iP, int i_rebin = 2, double iMinEnergy = -10. );
        void                             getEnergyResolutionPlot( TH2D* iP, double iMinEnergy = -10. );
        void                             getEnergyResolutionPlot68( TH2D* iP, double iMinEnergy = -10., double iReferenceValue = -999. );
        
        bool                             initializeIRFData();
        
    public:
    
        string fFile;
        string fA_MC;
        
        //////////////////////////////////
        // conditions
        //////////////////////////////////
        double fZe;
        double fWoff;
        int    fAzbin;
        double fIndex;
        int    fNoise;
        
        float  fEnergyLinTeV_min;
        float  fEnergyLinTeV_max;
        
        //////////////////////////////////
        // data
        //////////////////////////////////
        // effective areas
        TGraphAsymmErrors* gEffArea_MC;
        TGraphAsymmErrors* gEffArea_Rec;
        TGraphAsymmErrors* gEffArea_Recp80;
        // effective area ratios
        TGraphAsymmErrors* gEffArea_MC_Ratio;
        TGraphAsymmErrors* gEffArea_Rec_Ratio;
        // energy spectra
        TH1D* hEmc;
        TH1D* hEcut;
        TH1D* hEcut_rec;
        TH1D* hEcutUW;
        TH1D* hEcut_recUW;
        // energy reconstruction matrix
        TH2D* hERecMatrix;
        TProfile* hERecMatrixProfile;
        TH2D* hERecMatrixCoarse;
        TH2D* hERecMatrixQC;
        TH2D* hERecMatrixCoarseQC;
        // e_rec/e_mc
        TH2D* hEsysMCRelative2D;
        TProfile* hEsysMCRelative;
        // 2D energy error distribution
        TH2D* hEsys;
        // energy resolution
        TGraphErrors* gEnergyResolution;
        // energy bias
        TGraphErrors* gEnergyBias_Mean;
        TGraphErrors* gEnergyBias_Median;
        TGraphErrors* gEnergyLogBias_Mean;
        TGraphErrors* gEnergyLogBias_Median;
        // angular resolution (filled for CTA only)
        TGraphErrors* gAngularResolution;
        TGraphErrors* gAngularResolution80;             // 80% containment radius for angular resolution
        // cut efficiencies
        vector< TH1D* > hCutEfficiency;
        vector< TH1D* > hCutEfficiencyRelativePlots;
        // weight histograms
        TH1D* hWeightedRate;
        
        // resolution graphs
        vector< string >                           fIRF_TreeNames;
        vector< VInstrumentResponseFunctionData* > fIRF_Data;
        
        //////////////////////////////////
        // plotting
        //////////////////////////////////
        string fPlotOption;
        int    fColor;
        int    fLineStyle;
        int    fMarkerStyle;
        string fLegend;
        
        VInstrumentResponseFunctionReader();
        ~VInstrumentResponseFunctionReader() {}
        
        bool calculateEffectiveAreaRatios( TGraphAsymmErrors* g0 );
        bool fillBiasHistograms( TH1F* h = 0, string iMeanOrMedian = "mean" );
        bool fillEffectiveAreasHistograms( TH1F* h = 0, string iContainmentRadius = "", TH1F* hMC = 0 );
        bool fillResolutionHistogram( TH1F* h = 0, string iContainmentRadius = "68", string iResolutionTreeName = "t_angular_resolution" );
        bool fillData();
        bool fillData( string iDataLine, int iDataID );
        bool fillData( string iFile, double iZe = 20., double iWoff = 0.5, int iAzBin = 0, double iIndex = 2.0, int iNoise = 200, string iA_MC = "A_MC" );
        TH2D* getRecvsMCEnergy()
        {
            return hEsysMCRelative2D;
        }
        TH2D* getMigrationMatrix()
        {
            return hERecMatrix;
        }
        bool isZombie()
        {
            return fIsZombie;
        }
        void setDebug( bool iDebug = true )
        {
            fDebug = iDebug;
        }
        void setEnergyRange( float iEmin_linTeV, float iEmax_linTeV )
        {
            fEnergyLinTeV_min = iEmin_linTeV;
            fEnergyLinTeV_max = iEmax_linTeV;
        }
        
        ClassDef( VInstrumentResponseFunctionReader, 9 );
};


#endif
