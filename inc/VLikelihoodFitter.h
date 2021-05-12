//! VLikelihoodFitter fit and plot spectral data using likelihood methods

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
#include <Math/GSLMinimizer.h>
#include <Math/Functor.h>
#include <Math/Factory.h>
#include "VEnergySpectrum.h"
#include "VEnergySpectrumfromLiterature.h"


// #ifdef __CINT__

// #pragma link off all globals;
// #pragma link off all classes;
// #pragma link off all functions;

#ifndef __VLikelihoodFitter_H_
#define __VLikelihoodFitter_H_

using namespace std;


class VLikelihoodFitter : public VEnergySpectrum
{
public:


    VLikelihoodFitter(string filename) : VEnergySpectrum(filename)

    {

        VEnergySpectrum::setEnergyBinning(0.05);
        VEnergySpectrum::combineRuns();
        setThresholdBias(0.15);
        // Standard Analysis Default
        VEnergySpectrum::setEnergyThresholdDefinition( 2, -1., 0.1 );
        // Getting the minimum threshold as definded in VEnergySpectrum
        double i_min_thresh = 9999;
        for (unsigned int i = 0; i < fRunList.size(); i ++ )
        {
            if ( fRunList[i].energyThreshold <= 0 )
            {
                continue;
            }

            if (i_min_thresh > fRunList[i].energyThreshold)
            {
                i_min_thresh = fRunList[i].energyThreshold;
            }
        }


        // (GM) maybe better names? Is this fFitMin_TeV or fFit_logTeV?
        // (GM) same for max
        // (GM) (need a test for negative values?)
        fFitMin_logTeV = TMath::Log10(i_min_thresh);
        fFitMax_logTeV = 2.;

        setNormalisationEnergyLinear(1.0);
        setModel(0, 1.0);
        setBinWidth(0.1);

        fExcludeRun.clear();

        // Default time range set to time range of data
        setMJDMinMax (fRunList[0].MJD - 0.0001,  fRunList[fRunList.size()-1].MJD );

        fConfidenceInterval = 0;
        fNContours = 100  ;

        fModel = 0;
        fModel_linear = 0;
        fModel_intrinsic = 0;
        fModel_intrinsic_linear = 0;
        fGlobalBestFitParameters = 0;

        fMinimizer = 0;
        fFitStatus = 0;
        fFitfunction = 0;
        fEnergySpectrum = 0;
        fLiteratureSpectra = 0;
        fEBLAnalysis = false;

        bStopOnLastOn = false;
        bStopOnLastOff = false;
        bStopOnLastModel = false;

        bValidLiterature = loadSpectraFromLiterature();
        fCrabID = 1;  // Default to Whipple 1998 (same as VFluxCalculator)
    }


    ~VLikelihoodFitter()
    {
      // VAnalysisUtilities
      closeFile();
    }

    bool initialize();




    // Setting the min and max energy rannge in log scale
    void setEnergyFitMinMaxLog(double i_min, double i_max)
    {
        // Making sure minimum energy is less than the maximum energy
        // Otherwise defaulting to the VEnergySpectrum default values
        if (i_min >= i_max)
        {
            cout << "VLikelihoodFitter::setEnergyFitMinMaxLinear Error invalid energy range \n"
                 << "\t\tFit min (" << i_min << "), fit max (" << i_max << ")\n"
                 << "\t\tUsing default values" << endl;

        }
        else
        {
            fFitMin_logTeV = i_min;
            fFitMax_logTeV = i_max;
        }

    }

    // Setting the min and max energy rannge in linear scale
    void setEnergyFitMinMaxLinear(double i_min, double i_max)
    {
        // Making sure minimum energy is less than the maximum energy
        // Otherwise defaulting to the VEnergySpectrum default values
        if ( (i_min >= i_max) || (i_min < 0) || (i_max < 0) )
        {
            cout << "VLikelihoodFitter::setEnergyFitMinMaxLinear Error invalid energy range \n"
                 << "\t\tFit min (" << i_min << "), fit max (" << i_max << ")\n"
                 << "\t\tUsing default values" << endl;
        }
        else
        {
            fFitMin_logTeV = TMath::Log10(i_min);
            fFitMax_logTeV = TMath::Log10(i_max);
        }
    }

    void setBinWidth( double i_BinWidth);

    // Setting the min and max MJD dates
    // Also sets a single time bin (for Variability analysis)
    void setMJDMinMax (double i_MJDMin, double i_MJDMax, bool resetTimeBinning = true)
    {

        fMJD_Min = i_MJDMin;
        fMJD_Max = i_MJDMax;

        // Time bin details
        // Defaulting to total analysis range to allow for general use
        if (resetTimeBinning)
        {
          fVarIndexTimeBins.clear();
          fVarIndexTimeBins.assign(2,0);
          fVarIndexTimeBins[0] = i_MJDMin;
          fVarIndexTimeBins[1] = i_MJDMax;
          fNRunsInBin.clear();
          fNRunsInBin.assign(1,fRunList.size());
        }
    }


    // Setting the threshold bias which is applied in the unfolding process
    void setThresholdBias(double i_thresh)
    {
        // Catching negative bias
        if (i_thresh < 0)
        {
            cout << "VLikelihoodFitter::setThresholdBias Warning only positive value accepted\n"
                 << "\t\tDefaulting to 0.2 " << endl;
        }
        fThresholdBias = i_thresh;
    }


    // Get Response Matrix for the ith run
    TH2F* getResponseMatrix(int i)
    {

        // Checking if index is in the range
        if ( (i < 0) || (i >= (int)fRunList.size()) )
        {
            cout << "VLikelihoodFitter::getResponseMatrix Error index out of range [0-" << fRunList.size() << "]" <<  endl;
            return 0;
        }
        else
        {
            return fResponseMatrixRebinned[i];
        }
    }

    // Return the On counts
    vector < vector <double> > getOnCounts()
    {
        return fOnCounts;
    }

    // Return the Off counts
    vector < vector <double> > getOffCounts()
    {
        return fOffCounts;
    }

    // Setting the source Model
    // ID is the model ID
    // ifENorm is the normalization energy
    // (defaults to that set by setNormalisationEnergyLinear )
    void setModel(int ID = 0,  double ifENorm = 0 );

    // Print runwise information
    void printRunInfo();
    // Plot runwise information (debugging tool)
    TCanvas *getRunPlots(unsigned int i_Entry);
    // Plot energy bias for a given run (debugging tool)
    TCanvas *plotEnergyBias(unsigned int i_Entry);

    // Plot Observed and model counts with residuals for best fit model
    TCanvas *getTotalCountsPlots();


    // Model predicted excess counts for a set of parameters
    vector < vector <double> > getModelPredictedExcess(vector <double> iParms);

    // Model predicted off counts for a set of parameters
    vector < vector <double> > getModelPredictedOff(vector <double> iParms);

    // Calculate the likelihood for a set of parameters
    double getLogL(vector <double>  parms);	// User friendly version

    // Get a 1-sigma error contour for parameters i, j
    TGraph* getContours(unsigned int i, unsigned int j);

    // Return the maximum likelihood estimate of the model
    TF1* getLikelihoodFit( bool bContours = false );


    // Return the live time for the included run
    double getLiveTime();

    /*
    * Get log(L_0)
    * L_0 is defined as the likelihood of a hypothetical perfect Model
    * The form of this model is unknown, we just take the results
    * f_perfect ( E ) = dNdE_observed
    */
    double getLogL0 ();

    /*
    * Calculate the Likelihood ratio:
    * -2 (log(l_max) - log(l_0))
    * ~ chi^2 (NDF = NBins - fNParms)
    */
    double getChi2 (vector <double> iParms);

    double getNDF ();

    // Set the Normalization energy
    void setNormalisationEnergyLinear(double iNormEnergy)
    {
        if ( iNormEnergy <= 0 )
        {
            cout << "VLikelihoodFitter::setNormalisationEnergyLinear Warning invalid normalization energy (" << iNormEnergy << " TeV)!"
                 << "\t\tAllowed range: > 0."
                 << "\t\tDefaulting to 1.0 TeV" << endl;
        }
        fENorm = iNormEnergy;
    }


    // Setting the intrinsic Source Model
    void setEBLAnalysis( bool isetanalysis )
    {
        fEBLAnalysis = isetanalysis;
    };



    // Return the confidince interval
    TGraphAsymmErrors* getConfidenceInterval()
    {
        if (!fConfidenceInterval)
        {
            cout <<"VLikeLihoodFitter::getConfidenceInterval Error Confidence interval not found... Have you called getLikelihoodFit?" << endl;
            return 0;
        }
        return fConfidenceInterval;
    }





    /*
    *		Get the best differential energy spectrum (model dependant)
    *		iBestFit is the assumed Model
    *		bPrintAll == False only save details of energy bins with a valid fit
    *		bPrintAll == True will print all bin details
    */

    TGraphAsymmErrors* getEnergySpectrum(TF1 *iBestFit, bool bPrintAll = false);



    // Print binwise details (debugging tool)
    void printCountsInBins();

    // Add date to be excluded
    void addExclusionDate(double i_MJDStart, double i_MJDStop);

    // Add runs to be excluded
    void excludeRun(int i_Run)
    {
        fExcludeRun.push_back(i_Run);
    }


    // Set the EBL opacity
    // Tau(E)
    void setEBLOpacity (TGraph* i_EBLOpacity);

    // Print details of the fit applied to each run
    void getRunwiseFitStatus(TF1 *i_fit);

    /*
    * Getting Profile Likelihood for best-fit model
    * par - spectral parameter
    * min - par_min
    * max - par_max
    * energybin - energy bin to scan (-1 is the total fit)
    */
    TF1 *getProfileLikelihood( unsigned int par, double min, double max, int energybin = -1, bool bDelta = true );

    vector <double> getEnergyBins()
    {
        return fEnergyBins;
    }


    // Return a vector of runwime on/off normalization
    vector <double> getAlpha()
    {
      vector <double> i_alpha;

      for (unsigned int i = 0 ; i < fRunList.size(); i++)
      {
        i_alpha.push_back(fRunList[i].alpha);
      }

      return i_alpha;
    }


    // Set which last count definition (on,off,model)
    void setLastCountDefinition (string def);
    // These are public for simulating spectra
    // Would be safer for a set/reset counts option?
    vector < vector <double> > fOnCounts;
    vector < vector <double> > fOffCounts;

    // Return the fit status
    int getFitStatus()
    {
      return fFitStatus;
    }


    // Return the maximum likelihood for an energy bin
    double getBinMaxLogLikelihood(unsigned int ibin )
    {
      if (ibin < fSpectralPoint_likelihood_max.size() )
      {
        return fSpectralPoint_likelihood_max[ibin];
      }
      else
      {
        cout << "VLikelihoodFitter::getBinMaxLikelihood Bin ID must be < "  << fSpectralPoint_likelihood_max.size() << endl;
        return 0;
      }
    }


    // Freeze a spectral Parameter to a given value
    bool fixParameter(unsigned int par, double value)
    {

      // Check if parameter is out of range
      if (par >= fNParms)
      {
        cout << "VLikelihoodFitter::fixParameter Parameter out of range [0," << fNParms -1 << "]" << endl;
        return false;
      }

      // Checking if fFixedParameters has any values in it
      if (fFixedParameters.size() == 0)
      {
        fFixedParameters.assign(fNParms, -999);
      }


      fFixedParameters[par] = value ;
      return true;
    }


    // Returning variability index:
    // I_Var = -2 log(L/L0)
    // L = Norm allowed to vary
    // L0 = fixed to best fit parameters
    double getVariabilityIndex(double i_delT, TF1 *i_bestFit, double i_mjdMin = -999, double i_mjdMax = -999, int iPrintStatus = 1, bool i_ul = false);

    // Might as well use the variability index as a light curve code...
    TGraphErrors *getLightCurve();

    vector <double> fLCFlux;
    vector <double> fLCFluxErr;
    vector <double> fLCMJD;
    vector <double> fLCMJDErr;
    vector <double> fLCFluxTS;
    vector <double> fLCFluxUL;
    vector < double > fEnergyBinCentres;

    vector <float> fSpectralPoint_TS;


    // return MJDs of each run
    vector <float> getRunwiseMJD()
    {

      vector <float> iMJD(fRunList.size());
      for (unsigned int i = 0 ; i <  fRunList.size(); i ++)
      {
         iMJD[i] = fRunList[i].MJD;
      }

      return iMJD;
    }

    void setCrabSpectrum(int id)
    {
      fCrabID = id;
    }

private:

    // Binning Information
    int fNEnergyBins;
    vector <double>fEnergyBins;
    double fEnergyBinWidth;

    // Set the binning to be used in the analysis
    bool setAnalysisBinning(int i_fNBins, vector <double> i_fBins);




    // Number of bins used
    int fNBinsFit;
    int fNBinsFit_Total;
    int fNBinsFit_runwise;

    // Fit details
    double fThresholdBias;
    double fENorm;
    double fMJD_Min;
    double fMJD_Max;

    // Excluded dates and runs
    vector < vector <double> > fExcludeMJD;
    vector <int> fExcludeRun;

    // Energy of last On/Off count
    vector <double> fLastOn;
    vector <double> fLastOff;
    // Bin index of the final count
    vector < int > fLastOnCount;
    bool bStopOnLastOn;
    bool bStopOnLastOff;
    bool bStopOnLastModel;



    // Model Information
    int fModelID; 				// ID choses model type
    TF1 *fModel;					// Model which is used in fitting procedure (log scale)
    TF1 *fModel_linear;		// Linear scale model
    TF1 *fModel_intrinsic;			// Intrinsic model (for use with EBL analysis)
    TF1 *fModel_intrinsic_linear;    // Intrinsic model (for use with EBL analysis) linear scale
    unsigned int fNParms;					// Number of model parameters
    vector <string> fParmName; 		// Names of the parameters
    double fFitMin_logTeV;				// Min energy of model (log scale)
    double fFitMax_logTeV;				// Max energy of model (log scale)
    bool fEBLAnalysis;						// Enable fitting to EBL absorbed model
    double *fGlobalBestFitParameters; //Global copy of the current best fit parameters
    vector <double> fFixedParameters; // Vector of frozen parameters and their values
    bool bValidLiterature;            // Valid spectral file read in
    int fCrabID;
    VEnergySpectrumfromLiterature* fLiteratureSpectra;

    map <string, vector< vector <double> > >fContourMap;

    TGraphAsymmErrors* fConfidenceInterval;
    TGraphAsymmErrors *fEnergySpectrum;

    // Totals
    vector <int> fTotalOn;
    vector <int> fTotalOff;
    double getMeanAlpha();    // Livetime weighted mean alpha


    // Data Vectors
    vector <TH1D*> fOnRebinnedHistograms;
    vector <TH1D*> fOffRebinnedHistograms;
    vector <TH2F*> fResponseMatrixRebinned;
    vector <TGraphAsymmErrors*> fMeanEffectiveAreaMC;


    // Calculate the likelihood for a set of parameters
    double getLogL_internal(const double *parms);	// Likelihood based on the total counts

    // Return the On Counting histogram
    vector <TH1D*> getCountingHistogramOn()
    {
        return fOnRebinnedHistograms;
    }

    // Return the Off Counts histogram
    vector <TH1D*> getCountingHistogramOff()
    {
        return fOffRebinnedHistograms;
    }




    // Runwise energy bias
    vector < vector <double> > fEnergyBias;


    // Calculating the profile Likelihood
    // returns log(L) - log(L_max)
    double profileLikelihood(Double_t *x, Double_t *par);


    // Calculated integral flux from model
    float* getIntegralFlux(double i_EMin, double i_EMax, TF1* i_Model, bool i_log = true, bool i_ul = false );


    // EBL analysis
    // EBL opacity Tau(E)
    TGraph *fEBLOpacityGraph;


    // Accessing Raw Data
    // getCountingHistogramRaw("on"/"off")
    vector <TH1D*> getCountingHistogramRaw(string onoff);

    // Get response matrix from .anasum.root file
    vector <TH2F*> getResponseMatrixRaw();

    // Save TH1D data as vectors
    vector < vector <double> > getCounts(vector <TH1D*> i_hTemp);

    // Getting Vector of effective areas
    // returns:
    // 		true  - MC (True) energy
    //		false - Rec (reconstructed) energy
    vector <TGraphAsymmErrors*> getEffectiveAreasMCFromFile();

    // Calculate the confidince interval
    TGraphAsymmErrors* calculateConfidenceInterval( double* i_covmat, TF1 *i_fitfunction, int i_model, int i_nparms);

    // Broken Power Law
    double brokenPowerLaw(Double_t *x, Double_t *parm);

    // Fit fuction that takes into account EBL attenuation
    double calculateIntrinsicSpectrum(Double_t *x, Double_t *parm);

    // Apply spectral fit to indivual bin
    float* getSpectralPoint(  double BinMin, double BinMax, double iE_Norm, TF1* iBestFit, bool bPrintAll = false);

    // Return the Crab flux
    // Whipple
    double getCrabFlux( double iF, double i_EMin, double i_EMax );

    // Check if run is excluded
    bool   isRunExcluded( int iRun );
    // Check if MJD is excluded
    bool   isMJDExcluded( double iMJD );


    // Initialize the Minimizer
    bool initializeMinimizer( double iNormGuess = 1.e-12, int iPrintStatus = 1, bool iFixShape = false );

    // Runwise fit details
    double getRunwiseLogL(int i_run, vector <double> iParms);
    double getRunwiseLogL0(int i_run, vector <double> iParms);
    int getRunWiseFitInfo(int i_runnum, TF1* i_fit);

    // Calculate the Test Statistic (TS)
    // Sigma ~ sqrt(TS)
    double getBinTS(double i_on, double i_off, double i_alpha);


    // For Spectral Points
    vector <int> fSpectralPoint_FitStatus;
    vector <float> fSpectralPoint_likelihood_max;

    // Minimizer and function to be minimized
    ROOT::Math::Minimizer* fMinimizer;
    ROOT::Math::Functor* fFitfunction;
    int fFitStatus;

    // For contour plot
    vector <double> fIContours;
    vector <double> fJContours;
    unsigned int fNContours;



    // Get the bin which the last count is in
    int getLastCount( vector <double> ivec );


    // Check if Object is valid
    bool isPointerValid(TGraphAsymmErrors* i_obj);
    bool isPointerValid(TH2F* i_obj);


    // Generic functions to sum the runwise counts
    vector <double> sumCounts( vector < vector <double> > i_countVector );
    double sumCounts( vector < double > i_countVector );

    vector <double> fVarIndexTimeBins;
    vector <int> fNRunsInBin;


    // Reset spectral point info vectors
    void clearSpectralPoints()
    {
        fSpectralPoint_TS.clear();
        fSpectralPoint_FitStatus.clear();
        fSpectralPoint_likelihood_max.clear();
    }

    // Reset histograms with counting info
    void resetCountingHistograms()
    {
        fOnRebinnedHistograms.clear();
        fOffRebinnedHistograms.clear();
        fResponseMatrixRebinned.clear();
        fOnCounts.clear();
        fOffCounts.clear();
        fTotalOn.clear();
        fTotalOff.clear();
        fLastOn.clear();
        fLastOff.clear();
    }

    bool loadSpectraFromLiterature();

};

#endif
