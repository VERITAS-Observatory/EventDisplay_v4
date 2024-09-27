//! VLikelihoodObject fit and plot spectral data using likelihood methods

// #include <iostream>
// #include <cmath>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <TTree.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraphAsymmErrors.h"
#include "TGraph2D.h"
#include "TLine.h"
#include "VMathsandFunctions.h"
#include "VHistogramUtilities.h"

#include "TRandom3.h"

// #include <Math/GSLMinimizer.h>
// #include <Math/Functor.h>
// #include <Math/Factory.h>
// #include "VEnergySpectrum.h"
// #include "VEnergySpectrumfromLiterature.h"


// #ifdef __CINT__

// #pragma link off all globals;
// #pragma link off all classes;
// #pragma link off all functions;

#ifndef __VLikelihoodObject_H_
#define __VLikelihoodObject_H_

using namespace std;


class VLikelihoodObject
{
	public:

		// Two constructor methods
		// From a combined file
		VLikelihoodObject( string filename, int run_number );
		// From a single file
		VLikelihoodObject( string filename );
		
		~VLikelihoodObject();

		// Whether or not the run is active
		bool fActive;

		void setActiveRun(bool isActive) { fActive = isActive; }
		
		// Likelihood and model counts related functions
		// Parameters are handled by likelihood handler class
		/////////////////////////////////////////////////////////

		// Model Excess/off counts are always useful to have 
		vector <double> getModelPredictedExcess();
		vector <double> getModelPredictedOff();

		// LogL/LogL0
		double getLogL();
		double getLogL0();
		// Number of degrees of freedom
		int getNDF();
		
		// Each object has a pointer to the global model
		void setModel( TF1 *i_model) {fModel = i_model;}


		// Energy Details
		/////////////////////////////////////////////////////////
		// Analysis energy range
		void setEnergyRange(double i_min, double i_max, bool is_linear);
		// Energy threshold
		// i_method = 0 // fixed energy at i_value with is_linear defining if TeV or log10(TeV) 
		// i_method = 1 // fraction of the max effective area. Fraction set at i_value
		// i_method = 2 // set as at a fixed value of the energy bias (bins > bias are excluded)
		void setEnergyThreshold(int i_method, double i_value, bool is_linear = false );

		// Getter functions
		//////////////////////////////////////////////////////////
		vector <double> getOnCounts() { return fOnCounts;}
		vector <double> getOffCounts() { return fOffCounts;}
		double getAlpha() {return fAlpha;}
		// Return a clone to avoid coruption
		TH2F* getResponseMatrix() {return (TH2F*)fResponseMatrixRebinned->Clone();}
		TGraphAsymmErrors* getEffectiveArea() {return (TGraphAsymmErrors*)fMeanEffectiveAreaMC->Clone();}

		double getMJD() { return fMJD;}
		double getLiveTime() { return fLiveTime;}
		double getElevation() { return fElevation;}
		double getAzimuth() { return fAzimuth;}
		double getEnergyThreshold() {return fEnergyThreshold;}
		double getDuration() {return fDuration;}
		double getDeadTimeFraction() {return fDeadTimeFraction;}
		int getRunNumber() {return fRunNumber;}
		
		
		// Set the energy binning
		void setBinning(double i_binWidth, double i_binMin = -1.5, double i_binMax = 2);

		vector <double> getEnergyBins () {return fEnergyBins;}
		vector <double> getEnergyBinCentres () {return fEnergyBinCentres;}

		// TCanvas *drawEffectiveArea(){
		// 	TCanvas* c1 = new TCanvas();
		// 	fMeanEffectiveAreaMC->Draw("APL");
		// 	return c1;
			
		// }

		VLikelihoodObject *fakeIt(TF1 *i_model, int i_run_num = -1, double i_mjd = -1);

		VLikelihoodObject  *clone();

		TCanvas *peak();

		int getNBins();

		
	private:

		// On and off counts for the run
		vector <double> fOnCounts;
		vector <double> fOffCounts;
		// on/off normalization
		double fAlpha;

		// Counts histograms
		//////////////////////////////////////////////////////////
		// Stored for easy rebinning 
		TH1D* fOnHistogram;
		TH1D* fOffHistogram;
		TH1D* fOnHistogramRebinned;
		TH1D* fOffHistogramRebinned;
		
		// Model
		// This should be a strictly private pointer to the model
		// Keep private to make sure the model is thread safe
		// Model interface should be handled by the likelihood fitting class
		TF1 *fModel;				

		// IRFs
		//////////////////////////////////////////////////////////
		// Anasum file
		TFile* fAnaFile;

		// Respose matrix
		TH2F* fResponseMatrix;
		TH2F* fResponseMatrixRebinned;
		// Effective Areas (MC, True energy)
		TGraphAsymmErrors* fMeanEffectiveAreaMC;
		// Energy bias
		vector <double> fEnergyBias;

		// Binning Information
		int fNEnergyBins;
		vector <double>fEnergyBins;
		vector <double> fEnergyBinCentres;
		double fEnergyBinWidth;

		// Set the binning to be used in the analysis
		bool setAnalysisBinning( int i_fNBins, vector <double> i_fBins );

		// Energy Threshold and Limits
		double fEnergyThreshold;
		double fThresholdBias;
		double fEnergyMin;
		double fEnergyMax;
		

		// Runwise details that one might want to make cuts on
		//////////////////////////////////////////////////////////
		int fRunNumber;				// Run number
		double fMJD;				// MJD of the run
		double fDuration;			// Duration of the run
		double fDeadTimeFraction;	// Dead time fraction
		double fLiveTime;			// Dead time corrected exposure
		double fElevation;			// Elevation of the run
		double fAzimuth;;			// Azimuth; of the run


		// File I/O
		//////////////////////////////////////////////////////////
		TGraphAsymmErrors* getEffectiveAreasMCFromFile();
		TH2F* getResponseMatrixFromFile();
		void initialize(string filename, int indx);

		

		// Msc Helper functions
		//////////////////////////////////////////////////////////

		// Check if Object is valid
		bool isPointerValid( TGraphAsymmErrors* i_obj );
		bool isPointerValid( TH2F* i_obj );
		
		// File I/O Getters
		TH1D* getCountingHistogramRaw( string onoff );
		TH2F* getResponseMatrixRaw();

		// Get counts from a histogram
		vector <double> getCounts( TH1D* i_hTemp );

		// Delete any interal pointers
		void clearPointers();

};

#endif
