//!< VEffectiveAreaCalculator calculate effective areas and energy spectra

#ifndef VEffectiveAreaCalculator_H
#define VEffectiveAreaCalculator_H

#include "CData.h"
#include "VGammaHadronCuts.h"
#include "VAnaSumRunParameter.h"
#include "VASlalib.h"
#include "VEffectiveAreaCalculatorMCHistograms.h"
#include "TEfficiency.h"
#include "VHistogramUtilities.h"
#include "VInstrumentResponseFunctionRunParameter.h"
#include "VStatistics.h"
#include "VSpectralWeight.h"
#include "VUtilities.h"

#include "TChain.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TGraph2DErrors.h"
#include "TGraphAsymmErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TList.h"
#include "TMath.h"
#include "TProfile.h"
#include "TTree.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class VEffectiveAreaCalculator
{
    private:

	vector< vector< double > > fEffArea_time;
	vector< double > timebins;

        bool bNOFILE;
        TDirectory *fGDirectory;

        vector< double > fZe;
        vector< double > fMCZe;
        vector< vector< double > > fEff_WobbleOffsets;
        vector< vector< vector< double > > > fEff_Noise;
        vector< vector< vector< vector< double > > > > fEff_SpectralIndex;

// effective areas (reading of effective areas)
        unsigned int fNBins;
        vector< double > fEff_E0;
        map< unsigned int, vector< double > > fEffArea_map;

        vector< double >                      fEff_EsysMCRelative_EnergyAxis;
        map< unsigned int, vector< double > > fEff_EsysMCRelative;
        map< unsigned int, vector< double > > fEff_EsysMCRelativeE;
        unsigned int     fNMeanEffectiveArea;
	unsigned int     fNTimeBinnedMeanEffectiveArea;
        vector< double > fVMeanEffectiveArea;
	vector< double > fVTimeBinnedMeanEffectiveArea;
        TGraphAsymmErrors *gMeanEffectiveArea;
	TGraph2DErrors    *gTimeBinnedMeanEffectiveArea;

        TGraphErrors *gMeanSystematicErrorGraph;

// unique event counting
        map< unsigned int, unsigned short int> fUniqueEventCounter;

        vector< double > fAreaRadius;
        vector< string > fScatterMode;
        vector< double > fXWobble;                //!< wobble offset in camera coordinates (grisudet)
        vector< double > fYWobble;                //!< wobble offset in camera coordinates (grisudet)
        vector< int >    fNoise;
        vector< double > fPedVar;

        double fEnergyAxis_minimum_defaultValue;
        double fEnergyAxis_maximum_defaultValue;

	VInstrumentResponseFunctionRunParameter *fRunPara;

        VGammaHadronCuts *fCuts;
        bool fIgnoreEnergyReconstruction;
        bool fIsotropicArrivalDirections;
	bool fTelescopeTypeCutsSet;

// effective area calculation
        vector< double > fVMinAz;
        vector< double > fVMaxAz;
// spectral weighting
        vector< double > fVSpectralIndex;
	VSpectralWeight *fSpectralWeight;

// list of histograms
        vector< vector< TList* > > hList;

        vector< vector< TH1D* > > hVEmc;
        vector< vector< TH1D* > > hVEcut;
        vector< vector< TH1D* > > hVEcutLin;
        vector< vector< TH1D* > > hVEcutRec;
        vector< vector< TH1D* > > hVEcutUW;
        vector< vector< TH1D* > > hVEcutRecUW;
        vector< vector< TProfile* > > hVEmcSWeight;
        vector< vector< TH1D* > > hVEcut500;
        vector< vector< TProfile* > > hVEsysRec;
        vector< vector< TProfile* > > hVEsysMC;
        vector< vector< TProfile* > > hVEsysMCRelative;
        vector< vector< TH2D* > > hVEsysMCRelativeRMS;
        vector< vector< TH2D* > > hVEsysMCRelative2D;
        vector< vector< TH2D* > > hVEsys2D;
        vector< vector< TH2D* > > hVResponseMatrix;
        vector< vector< TProfile* > > hVResponseMatrixProfile;
        vector< vector< TH2D* > > hVResponseMatrixQC;
        vector< vector< TH2D* > > hVEmcCutCTA;
	vector< vector< TH2D* > > hVResponseMatrixFineQC;
	vector< vector< TH1D* > > hVWeightedRate;

        TList *hisTreeList;
        TH1D* hEmc;
        TH1D* hEcut;
        TH1D* hEcut500;
        TH1D* hEcutLin;
        TH1D* hEcutRec;
        TH1D* hEcutUW;
        TH1D* hEcutRecUW;
        TGraphAsymmErrors* gEffAreaMC;
        TGraphAsymmErrors* gEffAreaRec;
        TProfile* hEmcSWeight;
        TProfile* hEsysRec;
        TProfile* hEsysMC;
        TProfile* hEsysMCRelative;
	TH2D* hEsysMCRelativeRMS;
        TH2D* hEsysMCRelative2D;
        TH2D* hEsys2D;
        TH2D* hEmcCutCTA;
	TH2D* hResponseMatrixFineQC;
	TH2D* hResponseMatrix;
	TProfile* hResponseMatrixProfile;
	TH2D* hResponseMatrixQC;
	TH1D* hWeightedRate;
	vector< TH1D* > hEcutSub;                //! events after individual cuts

        int fEffectiveAreaVsEnergyMC;            // 0 = vs MC energy, 1 = vs rec energy (approx. method), 2 = vs rec energy (default)
        TTree *fEffArea;
        double ze;
        int fAzBin;                               //!< az bin: definitions see getEffectiveArea
        double fMinAz;
        double fMaxAz;
        double fXoff;
        double fYoff;
        double fWoff;
        double fSpectralIndex;
        int fTNoise;
        double fTNoisePE;
        double fTPedvar;
        int nbins;
        double e0[1000];
        double eff[1000];
        double seff_L[1000];
        double seff_U[1000];
        int Rec_nbins;
        double Rec_e0[1000];
        double Rec_eff[1000];
        double Rec_seff_L[1000];
        double Rec_seff_U[1000];

// effective area smoothing
        int fSmoothIter;
        double fSmoothThreshold;

        bool bEffectiveAreasareFunctions;
        bool bEffectiveAreasareHistograms;
	bool fClopperPearson;                    // statistic option for error calculation

// mean values from getEffectiveAreas
        double fEffectiveAreas_meanZe;
        double fEffectiveAreas_meanWoff;
        double fEffectiveAreas_meanPedVar;
        double fEffectiveAreas_meanIndex;
        double fEffectiveAreas_meanN;

// effective areas fit functions
        vector< TF1* > fEffAreaFitFunction;

	TGraphAsymmErrors* applyResponseMatrix( TH2* h, TGraphAsymmErrors *g );
        bool   binomialDivide( TGraphAsymmErrors *g, TH1D *hrec, TH1D *hmc );
        void   copyProfileHistograms( TProfile*,  TProfile* );
        void   copyHistograms( TH1*,  TH1*, bool );
        double getAzMean( double azmin, double azmax );
	double getCRWeight( double iEMC_TeV_log10 );
        bool   getEffectiveAreasFromFitFunction( TTree*, double azmin, double azmax, double ispectralindex );
        void   getEffectiveAreasFromFitFunction( unsigned int, unsigned int, double, double&, double& );
        double getEffectiveAreasFromHistograms( double erec, double ze, double woff, double iPedVar,
	                                        double iSpectralIndex, bool bAddtoMeanEffectiveArea = true,
						int iEffectiveAreaVsEnergyMC = 2 );
	bool   getMonteCarloSpectra( VEffectiveAreaCalculatorMCHistograms* );
	double getMCSolidAngleNormalization();
        vector< unsigned int > getUpperLowBins( vector< double > i_values, double d );
        bool   initializeEffectiveAreasFromHistograms( TTree *, TH1D*, double azmin, double azmax, double ispectralindex, double ipedvar );
        vector< double > interpolate_effectiveArea( double iV, double iVLower, double iVupper,
	                                            vector< double > iEL, vector< double > iEU, bool iCos = true );
        void   reset();
        void   smoothEffectiveAreas( map< unsigned int, vector< double > > );

    public:

        VEffectiveAreaCalculator( string ieffFile, double azmin, double azmax, double iPedVar, double iIndex,
	                          vector< double> fMCZe, int iSmoothIter = -1, double iSmoothThreshold = 1.,
				  int iEffectiveAreaVsEnergyMC = 2 );
	VEffectiveAreaCalculator( VInstrumentResponseFunctionRunParameter*, VGammaHadronCuts* );
        ~VEffectiveAreaCalculator();

        void cleanup();
        bool fill( TH1D *hE0mc, CData *d, VEffectiveAreaCalculatorMCHistograms *iMC_histo, unsigned int iMethod );
        TH1D*     getHistogramhEmc();
        TGraphErrors* getMeanSystematicErrorHistogram();
        TTree* getTree() { return fEffArea; }
	double getEnergyAxis_minimum_defaultValue() { return fEnergyAxis_minimum_defaultValue; }
	double getEnergyAxis_maximum_defaultValue() { return fEnergyAxis_maximum_defaultValue; }
        double getEffectiveArea( double erec, double ze, double iWoff, double iPedvar, double iSpectralIndex = -2.5,
	                         bool bAddtoMeanEffectiveArea = true, int iEffectiveAreaVsEnergyMC = 2 );
        TGraphAsymmErrors* getMeanEffectiveArea();
	TGraph2DErrors*    getTimeBinnedMeanEffectiveArea();
	void setTimeBinnedMeanEffectiveArea();
        void initializeHistograms( vector< double > iAzMin, vector< double > iAzMax, vector< double > iSpectralIndex );
        void resetHistograms( unsigned int iZe );
        void resetHistogramsVectors( unsigned int iZe );
        void setAzimuthCut( int iAzBin, double iAzMin, double iAzMax );
        void setEffectiveArea( int iMC ) { fEffectiveAreaVsEnergyMC = iMC; }
        void setIgnoreEnergyReconstructionCuts( bool iB = false ) { fIgnoreEnergyReconstruction = iB; }
        void setIsotropicArrivalDirections( bool iB = false ) { fIsotropicArrivalDirections = iB; }
        bool setMonteCarloEnergyRange( double iMin, double iMax, double iMCIndex = 2. );
        void setNoiseLevel( int iN, double iP );
	void setStatisticsOption( bool iClopperPearson = false ) { fClopperPearson = iClopperPearson; }
     	void setTelescopeTypeCuts( bool iB = true ) { fTelescopeTypeCutsSet = iB; }
        void setWobbleOffset( double x, double y );
	void resetTimeBin();
	void setTimeBin(double time);
};
#endif
