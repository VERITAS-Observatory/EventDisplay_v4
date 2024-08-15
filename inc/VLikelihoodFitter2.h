//! VLikelihoodFitter2 fit and plot spectral data using likelihood methods

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

#include "VLikelihoodObject.h"

#include <Math/GSLMinimizer.h>
#include <Math/Functor.h>
#include <Math/Factory.h>
// #include "VEnergySpectrum.h"
// #include "VEnergySpectrumfromLiterature.h"


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
		

		void setModel( int ID = 0,  double ifENorm = 0 );
		void setModel(TF1* i_model);

		bool setEnergyRange(double i_min, double i_max, bool is_linear);
		void setEnergyThreshold(int i_method, double i_value, bool is_linear = false );
		void setBinning(double i_binWidth, double i_binMin = -1.5, double i_binMax = 2);


		TF1 *fitEnergySpectrum();
		TGraphAsymmErrors *getSpectralPoints();
		void setTimeRange(double i_mjd_min = -1, double i_mjd_max = 999999);


		// Create a new object from the runs that are currently active
		VLikelihoodFitter2 fromActiveRuns();

		// LogL/LogL0
		double getLogL(vector <double>);
		double getLogL0();
		// Number of degrees of freedom
		int getNDF();

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


	private:

		// Model Parameters
		int fModelID;
		unsigned int fNParms;
		double fENorm;
		TF1 *fModel;
		TF1 *fModel_intrinsic;
		TF1 *fModel_linear;
		TF1 *fEBLOpacityGraph;
		bool fEBLAnalysis;


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
		bool fEnergyThreaholdBool;
		
		// Energy Binning
		double fEnergyBinWidth;
		double fEnergyBinMin;
		double fEnergyBinMax;

		vector <int> fExcludeRun;
		bool setRunExclusion(int i_run);

		// Initialize the Minimizer
		bool initializeMinimizer( double iNormGuess = 1.e-12, int iPrintStatus = 1, bool iFixShape = false );
		double getLogL_internal( const double* parms );	// Likelihood based on the total counts


		// Intrinsic model
		double calculateIntrinsicSpectrum( Double_t* x, Double_t* parm );


};
#endif
