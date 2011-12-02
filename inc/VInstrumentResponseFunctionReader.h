//! VInstrumentResponseFunctionReader data class for reading response functions (effective area, angular resolution, etc )
// Revision $Id: VInstrumentResponseFunctionReader.h,v 1.1.2.4 2011/02/03 12:49:09 gmaier Exp $

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
#include "VHistogramUtilities.h"
#include "VInstrumentResponseFunctionData.h"
#include "VPlotUtilities.h"

using namespace std;

class VInstrumentResponseFunctionReader : public VAnalysisUtilities, public VPlotUtilities, public VHistogramUtilities
{
   private:
    
    bool   fIsZombie;
    bool   fDebug;

    bool                             calculateCutEfficiencies();
    TGraphAsymmErrors*               calculateEffectiveAreaRatios( TGraphAsymmErrors *g0, TGraphAsymmErrors *g1 );
    VInstrumentResponseFunctionData* getIRFFromFile( TTree *, unsigned int );
    bool                             getDataFromFile();
    bool                             getDataFromCTAFile();
    void                             getEnergyResolutionPlot( TProfile *iP, int i_rebin = 2, double iMinEnergy = -10. );

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

//////////////////////////////////
// data
//////////////////////////////////
// effective areas
    TGraphAsymmErrors *gEffArea_MC;
    TGraphAsymmErrors *gEffArea_Rec;
// effective area ratios
    TGraphAsymmErrors *gEffArea_MC_Ratio;
    TGraphAsymmErrors *gEffArea_Rec_Ratio;
// energy spectra
    TH1D *hEmc;
    TH1D *hEcut;
    TH1D *hEcut_rec;
    TH1D *hEcutUW;
    TH1D *hEcut_recUW;
// energy reconstruction matrix
    TH2D *hERecMatrix;
// e_rec/e_mc
    TH2D *hEsysMCRelative2D;
// 2D energy error distribution
    TH2D *hEsys;
// energy resolution
    TGraphErrors *gEnergyResolution;
// energy bias
    TGraphErrors *gEnergySystematic_Mean;
    TGraphErrors *gEnergySystematic_Median;
// angular resolution (filled for CTA only)
    TGraphErrors *gAngularResolution;
    TGraphErrors *gAngularResolution80;
// cut efficiencies 
    vector< TH1D* > hCutEfficiency;
    vector< TH1D* > hCutEfficiencyRelativePlots;

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

    bool calculateEffectiveAreaRatios( TGraphAsymmErrors *g0 );
    bool fillEffectiveAreasHistograms( TH1F *h = 0, string iContainmentRadius = "", TH1F *hMC = 0 );
    bool fillResolutionHistogram( TH1F *h = 0, string iContainmentRadius = "68", string iResolutionTreeName = "t_angular_resolution" );
    bool fillData();
    bool fillData( string iDataLine, int iDataID );
    bool fillData( string iFile, double iZe = 20., double iWoff = 0.5, int iAzBin = 0, double iIndex = 2.4, int iNoise = 200, string iA_MC = "A_MC" );
    TH2D* getRecvsMCEnergy() { return hEsysMCRelative2D; }
    TH2D* getMigrationMatrix() { return hERecMatrix; }
    bool isZombie() { return fIsZombie; }
    void setDebug( bool iDebug = true ) { fDebug = iDebug; }

    ClassDef( VInstrumentResponseFunctionReader, 4 );
};


#endif
