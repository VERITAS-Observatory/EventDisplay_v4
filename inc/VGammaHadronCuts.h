//! VGammaHadronCuts: class for parameter cut definitions
//  Revision $Id: VGammaHadronCuts.h,v 1.13.2.2.4.1.12.2.6.2.2.5.6.1.2.1.2.15.4.4 2011/04/01 12:11:59 gmaier Exp $

#ifndef VGammaHadronCuts_H
#define VGammaHadronCuts_H

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "CData.h"
#include "VAnalysisUtilities.h"
#include "VGammaHadronCutsStatistics.h"
#include "VInstrumentResponseFunctionData.h"
#include "VTMVAEvaluator.h"
#include "VUtilities.h"

#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TSystem.h"
#include "TTree.h"

#define VANACUTS_PROBSELECTIONCUTS_MAX 1000

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// class for telescope type dependent multiplicity  cut
////////////////////////////////////////////////////////////////////////////////
class VNTelTypeCut : public TNamed
{
   public:

   vector< unsigned int > fTelType_counter;
   unsigned int           fNTelType_min;

   VNTelTypeCut();
  ~VNTelTypeCut() {}
   void print();
   bool test( CData* );

   ClassDef( VNTelTypeCut, 1 );
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
   dummy class (for compatibility reasons)

*/

class VGammaHadronCutsStats : public TNamed
{
   private:

   vector< string > fName;

   public:

   vector< unsigned int > fN;

   VGammaHadronCutsStats() {}
  ~VGammaHadronCutsStats() {}

   void printCutStatistics() {}
   void reset() {}

   ClassDef( VGammaHadronCutsStats, 2 );
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class VGammaHadronCuts : public VAnalysisUtilities
{
    private:
        
	bool   fDebug;                               // lots of debug output

        CData *fData;                                       //! transient
        string fDataDirectory;

// cut selector
        int fGammaHadronCutSelector;                            // see description at beginning of VGammaHadronCuts.cpp
	int fDirectionCutSelector;

// array characteristics (number of telescopes, centre of array)
        unsigned int fNTel;
        double       fArrayCentre_X;
        double       fArrayCentre_Y;

// number of possible telescope combinations
        unsigned int fNLTrigs;

// values calculated from shower/image parameter 
        double fMeanImageDistance;
        double fMeanImageLength;
        double fMeanImageWidth;

// event by event cuts (read in from an additional friend tree, used by random forest analysis, pulsar analysis, etc)
        TFile *fProbabilityCut_File;                  //!
        TTree *fProbabilityCut_Tree;                  //!
        int fProbabilityCut_QualityFlag;              // quality flag for probability cut 0: cut estimation failed; >0 successful probability estimation
        unsigned int fProbabilityCut_NSelectors;      // number of elements in fProbabilityCut_SelectionCut[]
        unsigned int fProbabilityCut_ProbID;          // array element to be used from fProbabilityCut_SelectionCut[]
        double fProbabilityCut_SelectionCut[VANACUTS_PROBSELECTIONCUTS_MAX];    // selection cut

// TMVA evaluator
        VTMVAEvaluator *fTMVAEvaluator;                             //!
	string          fTMVA_MVAMethod;
	string          fTMVAWeightFile;
	unsigned int    fTMVAWeightFileIndex_min;
	unsigned int    fTMVAWeightFileIndex_max;
	map< unsigned int, double > fTMVASignalEfficiency;
	map< unsigned int, double > fTMVA_MVACut;
	double          fTMVAProbabilityThreshold;
	string          fTMVAOptimizeSignalEfficiencyParticleNumberFile;
	double          fTMVAOptimizeSignalEfficiencySignificance_Min;
	double          fTMVAOptimizeSignalEfficiencySignalEvents_Min;
	double          fTMVAOptimizeSignalEfficiencyObservationTime_h;
	double          fTMVAFixedSignalEfficiencyMinEnergy;
	double          fTMVAFixedSignalEfficiencyAboveMinEnergy;
	TGraph*         fTMVABoxCut_Theta2_max;                
	double          fTMVA_EvaluationResult;

// orbital phase analysis
	TFile *fPhaseCut_File;                                      //!
	TTree *fPhaseCut_Tree;                                      //!
	double fOrbitalPhase;

// parameters for energy dependent theta2 cuts
// (implemented for MC only)
        string fFileNameAngRes;
        TFile *fFileAngRes;                                         //!
        string fF1AngResName;
        TF1   *fF1AngRes;
        double       fAngRes_ScalingFactor;
        double       fAngRes_AbsoluteMinimum;
	double       fAngRes_AbsoluteMaximum;
	unsigned int fAngResContainmentProbability;
// energy dependent theta2 cuts from IRF file
	TGraphErrors *fIRFAngRes;


// cut statistics
        VGammaHadronCutsStatistics* fStats;                       //!

        bool   applyProbabilityCut( int i, bool fIsOn);
	bool   applyFrogsCut( int i, bool fIsOn );
        bool   initAngularResolutionFile();
        bool   initPhaseCuts( int irun );
        bool   initPhaseCuts( string iDir );
        bool   initProbabilityCuts( int irun );
        bool   initProbabilityCuts( string iDir );
	bool   initTMVAEvaluator( string iTMVAFile, unsigned int iTMVAWeightFileIndex_min, unsigned int iTMVAWeightFileIndex_max );

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    public:


// cut variable values
// mono cuts
        double fAlpha_min;
        double fAlpha_max;
        double fDistance_min;
        double fDistance_max;
        double fLos_min;
        double fLos_max;
        double fLength_min;
        double fLength_max;
        double fWidth_min;
        double fWidth_max;
        double fAsymm_min;
        double fAsymm_max;
        double fSize_min;
        double fSize_max;
// stereo cuts
        double fCut_MeanImageDistance_min;
        double fCut_MeanImageDistance_max;
        double fCut_MeanImageLength_min;
        double fCut_MeanImageLength_max;
        double fCut_MeanImageWidth_min;
        double fCut_MeanImageWidth_max;

        double fCut_Theta2_min;
        double fCut_Theta2_max;
        double fCut_Chi2_min;
        double fCut_Chi2_max;
        double fCut_Size_min;
        double fCut_Size_max;
        int    fCut_Ntubes_min;
        int    fCut_Ntubes_max;
        double fCut_MSCW_min;
        double fCut_MSCW_max;
        double fCut_MSCL_min;
        double fCut_MSCL_max;
        double fCut_MSW_min;
        double fCut_MSW_max;
        double fCut_MSL_min;
        double fCut_MSL_max;
        vector< int > fCut_ImgSelect;
        double fCut_CameraFiducialSize_min;
        double fCut_CameraFiducialSize_max;
        bool   bMCCuts;
        double fCut_CameraFiducialSize_MC_min;
        double fCut_CameraFiducialSize_MC_max;
        double fCut_AverageCoreDistanceToTelescopes_min;
        double fCut_AverageCoreDistanceToTelescopes_max;
	double fCut_MinimumCoreDistanceToTelescopes_max;
        double fCut_dE_min;
        double fCut_dE_max;
        double fCut_EChi2_min;
        double fCut_EChi2_max;
        double fCut_Erec_min;
        double fCut_Erec_max;
        double fCut_Emmission_min;
        double fCut_Emmission_max;
        int    fCut_NImages_min;
        int    fCut_NImages_max;
	double fCut_CoreDistanceToArrayCentreX_min;
	double fCut_CoreDistanceToArrayCentreX_max;
	double fCut_CoreDistanceToArrayCentreY_min;
	double fCut_CoreDistanceToArrayCentreY_max;
	double fCut_CoreDistanceEdgeSize;
        double fCut_SizeSecondMax_min;
        double fCut_SizeSecondMax_max;
        double fProbabilityCut;
        vector <double> fProbabilityCutRangeLower;
        vector <double> fProbabilityCutRangeUpper;
        	  
        vector< VNTelTypeCut* > fNTelTypeCut;

	double frogsGoodnessImgCut;

	bool   fUseOrbitalPhaseCuts;
	double fOrbitalPhase_min;
	double fOrbitalPhase_max;

        VGammaHadronCuts();
       ~VGammaHadronCuts();

        bool   applyDirectionCuts( unsigned int iEnergyReconstructionMethod = 0, bool bCount = false, double x0 = -99999., double y0 = -99999. );
        bool   applyEnergyReconstructionQualityCuts( unsigned int iEnergyReconstructionMethod = 0, bool bCount = false );
        bool   applyInsideFiducialAreaCut( bool bCount = false );
        bool   applyInsideFiducialAreaCut( float Xoff, float Yoff, bool bCount = false );
        bool   applyMCXYoffCut( double x, double y, bool bCount = false );
	bool   applyMeanReducedScaledStereoShapeCuts();
	bool   applyMeanStereoShapeCuts();
        bool   applyMeanScaledStereoShapeCuts();
	bool   applyPhaseCut(int i);
        bool   applyShowerCoreCuts( bool iMC = false );
        bool   applyStereoQualityCuts( unsigned int iEnergyReconstructionMethod = 0, bool bCount = false, int iEntry = 0, bool fIsOn = false );
        bool   applyStereoShapeCuts();
	bool   applyTMVACut( int i );
        bool   applyTelTypeTest( bool bCount = false );
        TF1*   getAngularResolutionFunction() { return fF1AngRes; }
        double getAngularResolutionAbsoluteMinimum() { return fAngRes_AbsoluteMinimum; }
        double getAngularResolutionScaleFactor() { return fAngRes_ScalingFactor; }
        double getArrayCentre_X() { return fArrayCentre_X; }
        double getArrayCentre_Y() { return fArrayCentre_Y; }
	int    getDirectionCutSelector() { return fDirectionCutSelector; }
	int    getGammaHadronCutSelector() { return fGammaHadronCutSelector; }
        double getMeanImageDistance() { return fMeanImageDistance; }
        double getMeanImageLength() { return fMeanImageLength; }
        double getMeanImageWidth() { return fMeanImageWidth; }
	unsigned int getAngularResolutionContainmentRadius() { return fAngResContainmentProbability; }
        double getProbabilityCut_Selector( unsigned int iID = 0 ) { if( iID < fProbabilityCut_NSelectors ) return fProbabilityCut_SelectionCut[iID]; else return -1; }
        double getProbabilityCutAlpha(bool fIsOn);
        double getTheta2Cut_min( double e = 0.1 ) { if( e > 0. ) return fCut_Theta2_min; else return 0.; }
        double getTheta2Cut_max( double e );                           // get theta2 max cut (might be energy dependent)    [TeV] energy (linear)
	TGraph* getTheta2Cut_TMVA_max()  { return fTMVABoxCut_Theta2_max; }
	TGraph* getTheta2Cut_IRF_Max() { return fIRFAngRes; }
	double getTMVA_EvaluationResult() { return fTMVA_EvaluationResult; }
        void   initialize();
        bool   isGamma( int i = 0, bool bCount = false, bool fIsOn = true);
        bool   isMCCuts() { return bMCCuts; }
        void   newEvent( bool iFillStats = true );
        void   printCutSummary();
        void   printCutStatistics() { if( fStats ) fStats->printCutStatistics(); }
	void   printDirectionCuts();
	void   printSignalEfficiency();
	void   printTMVA_MVACut();
        bool   readCuts(string i_cutfilename, int iPrint = 1 );
        void   resetCutValues();
	void   resetCutStatistics();
        void   initializeCuts( int irun = -1, string iDir = "" );
        void   setArrayCentre( double iX = 0., double iY = 0. ) { fArrayCentre_X = iX; fArrayCentre_Y = iY; }
        void   setDataDirectory( string id ) { fDataDirectory = id; }
        bool   setDataTree( CData* idata );
	void   setDebug( bool iB = false ) { fDebug = iB; }
        void   setEnergyCuts( double imin, double imax ) { fCut_Erec_min = imin; fCut_Erec_max = imax; }
	bool   setIRFGraph( TGraphErrors *g );
        void   setNTel( unsigned int itel,  double iX = 0., double iY = 0. ) { fNTel = itel; fArrayCentre_X = iX; fArrayCentre_Y = iY; }
//        void   setShowerCoreCuts( double xmin, double xmax, double ymin, double ymax, double iEdge = -1. );
        void   setTheta2Cut( double it2 ) { fCut_Theta2_max = it2; }
	void   terminate();
	bool   useOrbitalPhaseCuts() { return fUseOrbitalPhaseCuts; }

        ClassDef(VGammaHadronCuts,38);
};
#endif
