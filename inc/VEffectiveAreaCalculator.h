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
#include "TMinuit.h"

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
		vector< vector< double > > fEffAreaMC_time;
		vector< double > timebins;
                unsigned int fObservatory;		
		
		float fMC_ScatterArea;
		
		bool bNOFILE;
		TDirectory* fGDirectory;
		
		TFile *fCloneTreeFile;
		vector< double > fZe;
		vector< double > fMCZe;
		vector< vector< double > > fEff_WobbleOffsets;
		vector< vector< vector< double > > > fEff_Noise;
		vector< vector< vector< vector< double > > > > fEff_SpectralIndex;
		
		// effective areas (reading of effective areas)
                unsigned int fBiasBin;
		unsigned int fNBins;
		unsigned int fhistoNEbins;
		vector< double > fEff_E0;
		map< unsigned int, vector< double > > fEffArea_map;
		map< unsigned int, vector< double > > fEffAreaMC_map;
		map< unsigned int, unsigned int > fEntry_map;

		map< unsigned int, vector< double > > fResMat_MC_map;
		map< unsigned int, vector< double > > fResMat_Rec_map;
		map< unsigned int, vector< double > > fResMat_Rec_Err_map;

		vector< double >                      fEff_EsysMCRelative_EnergyAxis;
		map< unsigned int, vector< double > > fEff_EsysMCRelative;
		map< unsigned int, vector< double > > fEff_EsysMCRelativeE;
		unsigned int     fNMeanEffectiveArea;
		unsigned int     fNMeanEffectiveAreaMC;
		unsigned int     fNMeanResponseMatrix;

		unsigned int     fNTimeBinnedMeanEffectiveArea;
		unsigned int     fNTimeBinnedMeanEffectiveAreaMC;

		vector< double > fVMeanEffectiveArea;
		vector< double > fVMeanEffectiveAreaMC;

		vector< double > fVTimeBinnedMeanEffectiveArea;
		vector< double > fVTimeBinnedMeanEffectiveAreaMC;

		TGraphAsymmErrors* gMeanEffectiveArea;
		TGraph2DErrors*    gTimeBinnedMeanEffectiveArea;
		
		TGraphAsymmErrors* gMeanEffectiveAreaMC;
		TH2D*			   hMeanResponseMatrix;
		TGraphErrors* gMeanSystematicErrorGraph;
		
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
		
		VInstrumentResponseFunctionRunParameter* fRunPara;
		
		VGammaHadronCuts* fCuts;
		bool fIgnoreEnergyReconstruction;
		bool fIsotropicArrivalDirections;
		bool fTelescopeTypeCutsSet;
		
		// effective area calculation
		vector< double > fVMinAz;
		vector< double > fVMaxAz;
		// spectral weighting
		vector< double > fVSpectralIndex;
		VSpectralWeight* fSpectralWeight;
		
		// list of histograms
		vector< vector< TList* > > hList;
		
		vector< vector< TH1D* > > hVEmc;
		vector< vector< TH1D* > > hVEcut;
		vector< vector< TH1D* > > hVEcutLin;
		vector< vector< TH1D* > > hVEcutNoTh2;
		vector< vector< TH1D* > > hVEcutRec;
		vector< vector< TH1D* > > hVEcutUW;
		vector< vector< TH1D* > > hVEcutRecUW;
		vector< vector< TH1D* > > hVEcutRecNoTh2;
		vector< vector< TProfile* > > hVEmcSWeight;
		vector< vector< TH1D* > > hVEcut500;
		vector< vector< TProfile* > > hVEsysRec;
		vector< vector< TProfile* > > hVEsysMC;
		vector< vector< TProfile* > > hVEsysMCRelative;
		vector< vector< TH2F* > > hVEsysMCRelativeRMS;
		vector< vector< TH2F* > > hVEsysMCRelative2D;
                vector< vector< TH2F* > > hVEsysMCRelative2DNoDirectionCut;
		vector< vector< TH2F* > > hVEsys2D;
		vector< vector< TH2D* > > hVResponseMatrix;
		vector< vector< TH2D* > > hVResponseMatrixFine;
		vector< vector< TProfile* > > hVResponseMatrixProfile;
		vector< vector< TH2D* > > hVResponseMatrixQC;
		vector< vector< TH2D* > > hVEmcCutCTA;
		vector< vector< TH2D* > > hVResponseMatrixFineQC;
                vector< vector< TH2D* > > hVResponseMatrixNoDirectionCut;
                vector< vector< TH2D* > > hVResponseMatrixFineNoDirectionCut;
                vector< vector< TH2D* > > hVAngErec2D;            // direction reconstruction
                vector< vector< TH2D* > > hVAngMC2D;            // direction reconstruction

		vector< vector< TH1D* > > hVWeightedRate;
		vector< vector< TH1D* > > hVWeightedRate005;
		vector< vector< vector < TH1D* > > > hVEcutSub;
		
                // angular resolution graphs (vector in az)
                vector< TGraphErrors* > fGraph_AngularResolution68p;
                vector< TGraphErrors* > fGraph_AngularResolution80p;
                /*
                vector< vector< TH2F* > > hVAngularDiff_2D;
                vector< vector< TH2F* > > hVAngularDiffEmc_2D;
                vector< vector< TH2F* > > hVAngularLogDiff_2D;
                vector< vector< TH2F* > > hVAngularLogDiffEmc_2D;
                */
                vector< TH2F* > hVAngularDiff_2D;
                vector< TH2F* > hVAngularDiffEmc_2D;
                vector< TH2F* > hVAngularLogDiff_2D;
                vector< TH2F* > hVAngularLogDiffEmc_2D;

                // written to the EffArea tree
		TList* hisTreeList;
		TH1D* hEmc;
		TH1D* hEcut;
		TH1D* hEcut500;
		TH1D* hEcutLin;
		TH1D* hEcutRec;
		TH1D* hEcutUW;
		TH1D* hEcutRecUW;
		TH1D* hEcutNoTh2;
		TH1D* hEcutRecNoTh2;
		TGraphAsymmErrors* gEffAreaMC;
		TGraphAsymmErrors* gEffAreaRec;
                TGraphAsymmErrors* gEffAreaNoTh2MC;
                TGraphAsymmErrors* gEffAreaNoTh2Rec;
		TProfile* hEmcSWeight;
		TProfile* hEsysRec;
		TProfile* hEsysMC;
		TProfile* hEsysMCRelative;
		TH2F* hEsysMCRelativeRMS;
		TH2F* hEsysMCRelative2D;
		TH2F* hEsys2D;
		TH2D* hEmcCutCTA;
		TH2D* hResponseMatrixFine;
		TH2D* hResponseMatrixFineQC;
		TH2D* hResponseMatrix;
		TProfile* hResponseMatrixProfile;
		TH2D* hResponseMatrixQC;

                TH2F* hEsysMCRelative2DNoDirectionCut;
                TH2D* hResponseMatrixNoDirectionCut;
                TH2D* hResponseMatrixFineNoDirectionCut;

		TH1D* hWeightedRate;
		TH1D* hWeightedRate005;
		vector< TH1D* > hEcutSub;                //! events after individual cuts
		
                TH2F *hAngularDiff_2D;
                TH2F *hAngularDiffEmc_2D;
                TH2F *hAngularLogDiff_2D;
                TH2F *hAngularLogDiffEmc_2D;
		
		int fEffectiveAreaVsEnergyMC;            // 0 = vs MC energy, 1 = vs rec energy (approx. method), 2 = vs rec energy (default)
		bool bLikelihoodAnalysis;

		TTree* fEffArea;
		TTree* fEffTree;

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
		double effNoTh2[1000];
		int nbins_MC;
		double e0_MC[1000];
		double eff_MC[1000];
		double seff_L[1000];
		double seff_U[1000];
                
                float eff_error[1000];
                float effNoTh2_error[1000];
                float esys_rel[1000];

		int Rec_nbins;
		double Rec_e0[1000];
		double Rec_eff[1000];
		double Rec_effNoTh2[1000];
		double Rec_seff_L[1000];
		double Rec_seff_U[1000];
		int nbins_ResMat;
		double ResMat_MC[1000];
		double ResMat_Rec[1000];
		double ResMat_Rec_Err[1000];

                float Rec_eff_error[1000];
                float Rec_effNoTh2_error[1000];

                float Rec_angRes_p68[1000];
                float Rec_angRes_p80[1000];

		TTree* fAcceptance_AfterCuts_tree;       //Information for all the events after cuts to construct the background map
		double fXoff_aC;
		double fYoff_aC;
		double fXoff_derot_aC;
		double fYoff_derot_aC;
		double fErec;
		double fEMC;
		double fCRweight;                         // #/s/sr (the right unit for the ctools acceptance map) This normalise the map to the CR spectrum
		// Needs option ESPECTRUM_FOR_WEIGHTING to be turned on, which only make sense for CR
		bool fsolid_angle_norm_done;
		double fsolid_angle_norm;                   // solid angle normalisation needed for the CRweight filled in fAcceptance_AfterCuts_tree (for the histogram it is done later in VSensitivityCalculator)
		void Calculate_Bck_solid_angle_norm();
		
		
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
		
		TF1 *fGauss;
		double hres_binw;
        	double* hres_bins;
        	vector <double> hres_binc;
           	int hres_nbins;
		TGraphAsymmErrors* applyResponseMatrix( TH2* h, TGraphAsymmErrors* g );
		bool   binomialDivide( TGraphAsymmErrors* g, TH1D* hrec, TH1D* hmc );
		void   copyProfileHistograms( TProfile*,  TProfile* );
		void   copyHistograms( TH1*,  TH1*, bool );
                void   fillAngularResolution( unsigned int i_az, bool iContaintment_80p );
		double getAzMean( double azmin, double azmax );
		double getCRWeight( double iEMC_TeV_log10, TH1* h , bool for_back_map = false );
		bool   getEffectiveAreasFromFitFunction( TTree*, double azmin, double azmax, double ispectralindex );
		void   getEffectiveAreasFromFitFunction( unsigned int, unsigned int, double, double&, double& );
		double getEffectiveAreasFromHistograms( double erec, double ze, double woff, double iPedVar,
							double iSpectralIndex, bool bAddtoMeanEffectiveArea = true,
							int iEffectiveAreaVsEnergyMC = 2 );
		bool   getMonteCarloSpectra( VEffectiveAreaCalculatorMCHistograms* );
		double getMCSolidAngleNormalization();
		vector< unsigned int > getUpperLowBins( vector< double > i_values, double d );
		bool   initializeEffectiveAreasFromHistograms( TTree*, TH1D*, double azmin, double azmax, double ispectralindex, double ipedvar );
		vector< double > interpolate_effectiveArea( double iV, double iVLower, double iVupper,
				vector< double > iEL, vector< double > iEU, bool iCos = true );

                TH2D*  interpolate_responseMatrix( double iV, double iVLower, double iVupper, TH2D *iElower, TH2D *iEupper, bool iCos = true );
                void   multiplyByScatterArea( TGraphAsymmErrors* g );
		void   reset();
		void   smoothEffectiveAreas( map< unsigned int, vector< double > > );
		
	public:
	
		VEffectiveAreaCalculator( string ieffFile, double azmin, double azmax, double iPedVar, double iIndex,
								  vector< double> fMCZe, int iSmoothIter = -1, double iSmoothThreshold = 1.,
								  int iEffectiveAreaVsEnergyMC = 2, bool iLikelihoodAnalysis = false );
		VEffectiveAreaCalculator( VInstrumentResponseFunctionRunParameter*, VGammaHadronCuts* );
		~VEffectiveAreaCalculator();
		
		void cleanup();
		bool fill( TH1D* hE0mc, CData* d, VEffectiveAreaCalculatorMCHistograms* iMC_histo, unsigned int iMethod );
		TH1D*     getHistogramhEmc();
		TGraphErrors* getMeanSystematicErrorHistogram();
		TTree* getTree()
		{
			return fEffArea;
		}
		TTree* getAcceptance_AfterCuts()
		{
			return fAcceptance_AfterCuts_tree;
		}
		double getEnergyAxis_minimum_defaultValue()
		{
			return fEnergyAxis_minimum_defaultValue;
		}
		double getEnergyAxis_maximum_defaultValue()
		{
			return fEnergyAxis_maximum_defaultValue;
		}
		double getEffectiveArea( double erec, double ze, double iWoff, double iPedvar, double iSpectralIndex = -2.5,
								 bool bAddtoMeanEffectiveArea = true, int iEffectiveAreaVsEnergyMC = 2 );
		TGraphAsymmErrors* getMeanEffectiveArea();
		TGraph2DErrors*    getTimeBinnedMeanEffectiveArea();
		TGraphAsymmErrors* getMeanEffectiveAreaMC();



		void addMeanResponseMatrix( vector <double> i_emc, vector <double> i_erec , vector <double> i_erec_err );

		TH2D* getMeanResponseMatrix()
		{
                        VHistogramUtilities::normalizeTH2D_y(hMeanResponseMatrix);
/*                        if( hMeanResponseMatrix )
                        {
                            return (TH2D*)hMeanResponseMatrix->Clone();
                        } */
                        return hMeanResponseMatrix;
                        // return 0;
		}

		void setTimeBinnedMeanEffectiveArea();
		void setTimeBinnedMeanEffectiveAreaMC( double i_time );

		void initializeHistograms( vector< double > iAzMin, vector< double > iAzMax, vector< double > iSpectralIndex );
		void resetHistograms( unsigned int iZe );
		void resetHistogramsVectors( unsigned int iZe );
                void setAngularResolution2D( unsigned int i_az, vector< TH2D* > );
                void setAngularResolutionGraph( unsigned int i_az, TGraphErrors* g, bool iAngContainment_80p );

		void setAzimuthCut( int iAzBin, double iAzMin, double iAzMax );
		void setEffectiveArea( int iMC )
		{
			fEffectiveAreaVsEnergyMC = iMC;
		}
		void setIgnoreEnergyReconstructionCuts( bool iB = false )
		{
			fIgnoreEnergyReconstruction = iB;
		}
		void setIsotropicArrivalDirections( bool iB = false )
		{
			fIsotropicArrivalDirections = iB;
		}
		bool setMonteCarloEnergyRange( double iMin, double iMax, double iMCIndex = 2. );
		void setNoiseLevel( int iN, double iP );
		void setStatisticsOption( bool iClopperPearson = false )
		{
			fClopperPearson = iClopperPearson;
		}
		void setTelescopeTypeCuts( bool iB = true )
		{
			fTelescopeTypeCutsSet = iB;
		}
		void setWobbleOffset( double x, double y );
		void resetTimeBin();
		void setTimeBin( double time );
};
#endif
