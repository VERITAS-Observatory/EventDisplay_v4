//! VEnergySpectrum analyse and plot energy spectra
// Revision $Id: VEnergySpectrum.h,v 1.1.2.13.2.1.4.1.4.4.4.2 2011/04/11 16:09:05 gmaier Exp $

#ifndef VEnergySpectrum_h
#define VEnergySpectrum_h

#include "VAnalysisUtilities.h"
#include "VDifferentialFlux.h"
#include "VEnergyThreshold.h"
#include "VMathsandFunctions.h"
#include "VPlotUtilities.h"
#include "VSpectralFitter.h"
#include "VStatistics.h"

#include "TArrow.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TMath.h"
#include "TTree.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VEnergySpectrum : public VAnalysisUtilities, public VPlotUtilities
{
    private:

        bool  bZombie;                            //! status of files

        bool  bCombineRuns;                       //! rerun combine runs

        int  fTotalRun;

        string fDataSetName;                      //! name for histogram, canvases, etc. (no spaces, special characters)

// vector with differential flux values and upper limits
        vector< VDifferentialFlux > fDifferentialFlux;

        double fAnalysisMinEnergy;                // linear energy axis [TeV]
        double fAnalysisMaxEnergy;                // linear energy axis [TeV]
        double fAnalysisEnergyBinning;            // log energy axis [log10(TeV)]
        bool   fAnalysisHistogramAddingUseLowEdge;
        bool   bEnergyAxisLinear;

                                                  // 0 = max in diff spectrum, 1 = XX % systematic in energy reconstruction, 2 = fraction of maximum effective area
        unsigned int fAnalysisEnergyThresholdDefinition;
        double fAnalysisMaxEnergySystematic;      // maximum allowed systematic error in energy reconstruction
        double fAnalysisMaxEffectiveAreaFraction; // energy threshold is defined as fraction of maximum effective area

        double fAnalysisSignificance;
        double fAnalysisMinEvents;
        double fAnalysisUpperLimits;
        int    fAnalysisLiAndMaEquation;
        int    fAnalysisUpperLimitAlgorithm;

// spectral fit variables
        VSpectralFitter *fSpectralFitter;
        int    fSpectralFitFunction;              // 0: power law
                                                  // Default: 1 TeV
        double fSpectralFitFluxNormalisationEnergy;
        double fSpectralFitEnergy_min;
        double fSpectralFitEnergy_max;

// plotting variables
        TCanvas* fPlottingCanvas;
                                                  // spectral weight to calculate bin centers
        double fPlottingSpectralWeightForBinCenter;
        double fPlottingYaxisMin;
        double fPlottingYaxisMax;
        double fPlottingMultiplierIndex;
        double fPlottingMinEnergy;                // linear energy axis [TeV]
        double fPlottingMaxEnergy;                // linear energy axis [TeV]
        bool   fPlottingLogEnergyAxis;            // plot log or lin values on energy axis (default=true)
        TGraphErrors *gEnergySpectrum;
        TGraphErrors *gEnergySpectrumFitResiduals;

// rebinning of energy spectra
	TH1D *nRebinner;
	TH1D *nOriginalBinner;

	bool   bUseRebinner;
	vector< int > newBinningGroupings;
		  
        int    fPlottingEnergySpectrumFitLineColor;
        int    fPlottingEnergySpectrumFitLineStyle;
        float  fPlottingEnergySpectrumFitLineWidth;

        TF1   *fEnergySpectrumFit;

        void   addObservationTime( TH1* h, double iTObs, double iEThreshold, bool bLinearX = false );
        void   addHistogram( TH1*h1, TH1* h2, double iEThreshold, bool bLinearX );
        void   divideEnergySpectrumbydE( TH1* h, bool blin = false );
        void   rebinEnergySpectrum( TH1D* h, double iER, bool bLin = false );
        void   multiplyEnergySpectrumbydE( TH1* h, bool blin = false );
	TH1*   setVariableBinning(TH1 *a);
	void   setOriginalBinner(TH1 *a);

// histograms
        TH1D *hErec;
        TH1D *hErecCountsOn;
        TH1D *hErecCountsOff;
        TH1D *hErecTotalTime;

// definition of energy value in a log bin
        unsigned int fEnergyInBinDefinition;    // 0 = mean of (lin) energy, 1 = barycentric mean, 2 = spectral weighted mean

// energy thresholds
        string fEnergyThresholdFileName;
        double fEnergyThresholdFixedValue;

// total numbers
        double fTotalObservationTime;
        double fTotalNormalisationFactor;

// fill and plot energy spectrum graph
        TGraphErrors* plot_energySpectrum();

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
    public:

        VEnergySpectrum( string ifile, string iname = "E", int irun = -1  );
        ~VEnergySpectrum() {}

	double    calculateIntegralFlux( double iMinEnergy_TeV, double iMaxEnergy_TeV = 1.e6 );
        void      calculateDifferentialFluxes();
        bool      combineRuns();
        bool      combineRuns( vector< int > iRunList, bool blin = false );
        vector< VDifferentialFlux > getDifferentialFlux() { return fDifferentialFlux; }
        TH1D*     getEnergyHistogram() { return hErec; }
        TH1D*     getEnergyCountingOnHistogram() { return hErecCountsOn; }
        TH1D*     getEnergyCountingOffHistogram() { return hErecCountsOff; }
        TGraphErrors* getEnergySpectrumGraph();
        TH1D*     getTotalTimeHistogram() { return hErecTotalTime; }
        double    getTotalNormalisationFactor() { return fTotalNormalisationFactor; }
        bool      isZombie() { return bZombie; }
        void      printDifferentialFluxes( bool bSED = false );
        TCanvas*  plot( TCanvas *c = 0 );
        TCanvas*  plotCountingHistograms( TCanvas *c = 0 );
        void      plotEventNumbers( Double_t ts = 0.02 );
        void      plotFitValues();
        TCanvas*  plotResiduals( TCanvas *c = 0 );
        TCanvas*  plotLifeTimevsEnergy( TCanvas *c = 0 );

	void printEnergyBins();
        void setAddHistogramParameters( bool iB = false ) { fAnalysisHistogramAddingUseLowEdge = iB; }
        void setEnergyBinning( double iBin = 0.2 );
	bool setEnergyInBinDefinition( unsigned int iE = 2 );
	void setReBinningGroupings( vector< int > g ) { newBinningGroupings = g; }
	void setRebinBool(bool i_rebin);
        void setEnergyRangeLinear( double xmin, double max );
        void setEnergyRangeLog( double xmin, double xmax );
        void setEnergyThreshold( double a ) { fEnergyThresholdFixedValue = a; }
        void setEnergyThreshold( string a ) { fEnergyThresholdFileName = a; }
        void setEnergyThresholdDefinition( unsigned int iDef = 2, double iSys = 0.1, double iMaxEff = 0.1 )
	                                 { fAnalysisEnergyThresholdDefinition = iDef; fAnalysisMaxEnergySystematic = iSys; fAnalysisMaxEffectiveAreaFraction = iMaxEff; }
        void setSignificanceParameters( double iSig = 2., double iMinEvents = 5., double iUpperLimit = 0.99, int iLiAndMa = 5, int iULAlgo = 0 );

        TF1* fitEnergySpectrum( string iname = "fit", bool bDraw = true );
        void setSpectralFitFunction( int iD  = 0 ) { fSpectralFitFunction = iD; }
        void setSpectralFitFluxNormalisationEnergy( double iS = 1. ) { fSpectralFitFluxNormalisationEnergy = iS; }
        void setSpectralFitRangeLin( double xmin = 0.1, double xmax = 10. ) { fSpectralFitEnergy_min = xmin; fSpectralFitEnergy_max = xmax; }
        void setSpectralFitRangeLog( double xmin = 0.1, double xmax = 10. ) { fSpectralFitEnergy_min = TMath::Power( 10., xmin ); fSpectralFitEnergy_max = TMath::Power( 10., xmax ); }
        void setSpectralFitPlottingStyle( int iColor = 1, int iStyle = 1, float iWidth = 2. );

        TCanvas* getPlottingCanvas() { return fPlottingCanvas; }
        void setPlottingEnergyRangeLinear( double Emin_TeV = 0.05, double Emax_TeV = 20. ) { fPlottingMinEnergy = Emin_TeV; fPlottingMaxEnergy = Emax_TeV; }
        void setPlottingEnergyRangeLog( double xmin = -1.2, double xmax = 1.3 ) { fPlottingMinEnergy = TMath::Power( 10., xmin ); fPlottingMaxEnergy = TMath::Power( 10., xmax ); }
        void setPlottingLogEnergyAxis( bool iB = true ) { fPlottingLogEnergyAxis = iB; }
        void setPlottingMultiplierIndex( double iS = 0. ) { fPlottingMultiplierIndex = iS; }
        void setPlottingSpectralWeightForBinCenter( double iS = -2.5 ) { fPlottingSpectralWeightForBinCenter = iS; }
        void setPlottingYaxis( double iMin = 1.e-14, double iMax = 1.e-8 ) { fPlottingYaxisMin = iMin; fPlottingYaxisMax = iMax; }

        ClassDef(VEnergySpectrum,6);
};
#endif
