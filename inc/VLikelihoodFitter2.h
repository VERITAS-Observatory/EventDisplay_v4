//! VLikelihoodFitter2 fit and plot spectral data using likelihood methods

// #include <iostream>
// #include <cmath>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
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
#include "VEnergySpectrumfromLiterature.h"


#include "VLikelihoodObject.h"

#include <Math/GSLMinimizer.h>
#include <Math/Functor.h>
#include <Math/Factory.h>

// #include "ROOT/TProcessExecutor.hxx"
// #include "VEnergySpectrum.h"
// #include "VEnergySpectrumfromLiterature.h"

#include <omp.h>

// #ifdef __CINT__

// #pragma link off all globals;
// #pragma link off all classes;
// #pragma link off all functions;

#ifndef __VLikelihoodFitter2_H_
#define __VLikelihoodFitter2_H_

using namespace std;


class VLikelihoodFitter2
{
	public:
		VLikelihoodFitter2();
		~VLikelihoodFitter2();
		

		void loadFromFile(string filename);
		bool addRun(string filename);
		bool addRun(string filename, int i_runNumber);
		bool addObject(VLikelihoodObject* i_object);

		bool addFromCombined(string filename);


		// Setters
		// Set the model
		void setModel( int ID = 0,  double ifENorm = 0 );
		// Let the user set the model from a custom 
		// ToDo implment this with safety checks
		void setModel(TF1* i_model);

		// Set the number of threads
		void setNumThreads(int i_numThreads);

		bool setEnergyRange(double i_min, double i_max, bool is_linear);
		void setEnergyThreshold(int i_method, double i_value, bool is_linear = false );
		void setBinning(double i_binWidth, double i_binMin = -1.5, double i_binMax = 2);


		TF1 *fitEnergySpectrum(int verbosity = 1);
		TGraphAsymmErrors *getSpectralPoints();
		void setTimeRange(double i_mjd_min = -1, double i_mjd_max = 999999);


		// Create a new object from the runs that are currently active
		VLikelihoodFitter2 fromActiveRuns();

		// LogL/LogL0
		double getLogL(vector <double>);
		double getLogL0();
		// Number of degrees of freedom
		int getNDF();
		double getChi2( vector <double> i_parms, bool i_print = false );


		vector <VLikelihoodObject*> fLikelihoodObjects;

		// Add date to be excluded
		void addExclusionDate( double i_MJDStart, double i_MJDStop );

		// Add runs to be excluded
		void excludeRun( int i_run )
		{
			// Only exclude the run if it is in the runlist
			if (setRunExclusion(i_run)){
				fExcludeRun.push_back( i_run );
			}
		}

		// Set the Normalization energy
		void setNormalisationEnergyLinear( double iNormEnergy );

		// Set the Energy binning
		void setEnergyBinning(double i_binw, double i_binMin, double i_binMax);

		// Get the binned best fit spectral points
		TGraphAsymmErrors* getEnergySpectrum( TF1* iBestFit, bool bPrintAll = false );

		// Getters
		TF1* getModel() { return (TF1*)fModel->Clone(); }

		// Calculate the variability index
		double getVariabilityIndex( double i_delT, TF1* i_bestFit, double i_mjdMin = -999, double i_mjdMax = -999, int iPrintStatus = 1, bool i_ul = false );


		void setCrabSpectrum( int id ) {fCrabID = id;};

		TGraph* getProfileLikelihood( int i_parm, double i_min, double i_max, int i_nsteps, bool is_linear = false );
		vector <VLikelihoodFitter2*> getTimeBinnedData( double i_deltaT, double i_mjdMin, double i_mjdMax );
		vector <VLikelihoodFitter2*> getTimeBinnedData( vector <double> i_timeBins );

		int  getNRuns() { return fLikelihoodObjects.size(); }

		void fixParameter( int i_parm, double i_value );
	private:

		// Parallel
		int fNumThreads;


		// Model Parameters
		int fModelID;
		int fFitStatus;
		unsigned int fNParms;
		double fENorm;
		TF1 *fModel;
		TF1 *fModel_intrinsic;
		TF1 *fModel_linear;
		TF1 *fEBLOpacityGraph;
		bool fEBLAnalysis;
		vector <string> fParmName; 		// Names of the parameters
		vector <double> fFixedParameters;	// Fixed parameters
		double *fGlobalBestFitParameters;	// (Global) Best fit parameters
		
		// Minimizer
		ROOT::Math::Minimizer* fMinimizer;
		ROOT::Math::Functor* fFitfunction;
		// ROOT::TProcessExecutor* fExecutor;

		// Energy Spectrum and confidence intervals
		TGraphAsymmErrors* fConfidenceInterval;
		TGraphAsymmErrors* fEnergySpectrum;

		// Updaters
		// These are private functions to loop and update individual VLikelihoodObjects
		void updateModel();
		void updateEnergyRange();
		void updateEnergyBinning();
		void updateEnergyThreshold();

		// Other Misc Models
		double brokenPowerLaw( Double_t* x, Double_t* parm );

		
		// Range parameters (energy and mjd)
		double fEnergyMin;
		double fEnergyMax;
		double fEnergyMin_linear;
		double fEnergyMax_linear;
		double fMJDMin;
		double fMJDMax;

		// Energy Threshold
		int fEnergyThresholdMethod;
		double fEnergyThresholdValue;
		bool fEnergyThresholdBool;
		
		// Energy Binning
		double fEnergyBinWidth;
		double fEnergyBinMin;
		double fEnergyBinMax;
		int fNBinsFit_Total;


		// Energy Spectrum
		unsigned int fNEnergyBins;
		vector <double> fEnergyBins;
		vector <double> fEnergyBinCentres;
		vector <double> fSpectralPoint_likelihood_max;
		vector <double> fSpectralPoint_TS;
		vector <double> fSpectralPoint_FitStatus;

		vector <int> fExcludeRun;
		bool setRunExclusion(int i_run);

		// Initialize the Minimizer
		bool initializeMinimizer( double iNormGuess = 1.e-12, int iPrintStatus = 1, bool iFixShape = false );
		double getLogL_internal( const double* parms );	// Likelihood based on the total counts


		// Intrinsic model
		double calculateIntrinsicSpectrum( Double_t* x, Double_t* parm );

		// Confidince interval
		TGraphAsymmErrors*calculateConfidenceInterval( double* i_covmat, TF1* i_fitfunction, int i_model, int i_fNparms );


		// Get the spectral points
		float* getSpectralPoint( double BinMin, double BinMax, double ifENorm, TF1* iBestFit, bool bPrintAll );
		void clearSpectralPoints();


		// Light curve
		float* getIntegralFlux( double i_EMin, double i_EMax, TF1* i_Model, bool i_log = true, bool i_ul = false );
		double getCrabFlux( double iF, double i_EMin, double i_EMax );

		// Spectrum from literature
		bool bValidLiterature;            // Valid spectral file read in
		int fCrabID;
		VEnergySpectrumfromLiterature* fLiteratureSpectra;


		// Function to handle crab spectra from literature
		void loadSpectraFromLiterature( string filename = "" );


		// Get the active runs (not excluded)
		vector <VLikelihoodObject*> getActiveRuns();



};
#endif
