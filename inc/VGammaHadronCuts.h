//! VGammaHadronCuts: class for parameter cut definitions
//  Revision $Id: VGammaHadronCuts.h,v 1.13.2.2.4.1.12.2.6.2.2.5.6.1.2.1.2.15.4.4 2011/04/01 12:11:59 gmaier Exp $

#ifndef VGammaHadronCuts_H
#define VGammaHadronCuts_H

#include <iostream>
#include <fstream>
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
/*

      data class for telescope type dependent multiplicity  cut

*/
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

// array characteristics (number of telescopes, 
        unsigned int fNTel;
        double       fArrayCentre_X;
        double       fArrayCentre_Y;

// number of possible telescope combinations
        unsigned int fNLTrigs;

// values calculated from shower/image parameter 
        double fMeanDistance;
        double fMeanLength;
        double fMeanWidth;

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
	double          fTMVASignalEfficiency;
	double          fTMVAProbabilityThreshold;
	string          fTMVAOptimizeSignalEfficiencyParticleNumberFile;
	double          fTMVAOptimizeSignalEfficiencySourceStrengthCU;
	double          fTMVAOptimizeSignalEfficiencyObservationTime_h;
	TGraph*         fTMVABoxCut_Theta2_max;                

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
        double fAngRes_ScalingFactor;
        double fAngRes_AbsoluteMinimum;
	double fAngRes_AbsoluteMaximum;
	unsigned int fAngResContainmentProbability;
// energy dependent theta2 cuts from IRF file
	TGraphErrors *fIRFAngRes;

        bool initAngularResolutionFile();

// cut statistics
        VGammaHadronCutsStatistics* fStats;                       //!

        bool   applyProbabilityCut( int i, bool fIsOn);
	bool   applyTMVACut( int i, bool fIsOn );
	bool   applyFrogsCut( int i, bool fIsOn );
        bool   initPhaseCuts( int irun );
        bool   initPhaseCuts( string iDir );
        bool   initProbabilityCuts( int irun );
        bool   initProbabilityCuts( string iDir );
	bool   initTMVAEvaluator( string iTMVAFile, unsigned int iTMVAWeightFileIndex_min, unsigned int iTMVAWeightFileIndex_max );

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    public:


// cut variable values
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

        double fArrayDistance_min;
        double fArrayDistance_max;
        double fArrayTheta2_min;
        double fArrayTheta2_max;
        double fArrayChi2_min;
        double fArrayChi2_max;
        double fArrayLength_min;
        double fArrayLength_max;
        double fArrayWidth_min;
        double fArrayWidth_max;
        double fArrayAsymm_min;
        double fArrayAsymm_max;
        double fArraySize_min;
        double fArraySize_max;
        int    fArrayNtubes_min;
        int    fArrayNtubes_max;
        int    fArrayNTel_min;
        int    fArrayNTel_max;
        vector< int > fArrayLTrig;
        double fArrayMSCW_min;
        double fArrayMSCW_max;
        double fArrayMSCL_min;
        double fArrayMSCL_max;
        double fArrayMSW_min;
        double fArrayMSW_max;
        double fArrayMSL_min;
        double fArrayMSL_max;
        double fArrayxyoff_min;
        double fArrayxyoff_max;
        double fArrayCore_min;
        double fArrayCore_max;
        double fArraydE_min;
        double fArraydE_max;
        double fArrayEChi2_min;
        double fArrayEChi2_max;
        double fArrayErec_min;
        double fArrayErec_max;
        double fArrayEmmission_min;
        double fArrayEmmission_max;
        int    fArrayNImages_min;
        int    fArrayNImages_max;
        double fCoreX_min;
        double fCoreX_max;
        double fCoreY_min;
        double fCoreY_max;
        double fCoreEdge;

        double fArraySizeSecondMax_min;
        double fArraySizeSecondMax_max;

        double fProbabilityCut;
        vector <double> fProbabilityCutRangeLower;
        vector <double> fProbabilityCutRangeUpper;
        	  
        vector< VNTelTypeCut* > fNTelTypeCut;

        bool   bMCCuts;
        double fArrayxyoff_MC_min;
        double fArrayxyoff_MC_max;

	double frogsGoodnessImgCut;

	bool   fUseOrbitalPhaseCuts;
	double fOrbitalPhase_min;
	double fOrbitalPhase_max;

        VGammaHadronCuts();
       ~VGammaHadronCuts();

        bool   applyDirectionCuts( unsigned int iEnergyReconstructionMethod = 0, bool bCount = false, double x0 = -99999., double y0 = -99999. );
        bool   applyEnergyReconstructionQualityCuts();
        bool   applyEnergyReconstructionQualityCuts( unsigned int iEnergyReconstructionMethod, bool bCount = false );
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
        bool   applyTelTypeTest( bool bCount = false );

        void   initialize();

        bool   isGamma() { return isGamma( 0 ); }
        bool   isGamma( int i, bool bCount = false, bool fIsOn = true);

        bool   isMCCuts() { return bMCCuts; }

        void   newEvent( bool iFillStats = true );

        TF1*   getAngularResolutionFunction() { return fF1AngRes; }
        double getAngularResolutionAbsoluteMinimum() { return fAngRes_AbsoluteMinimum; }
        double getAngularResolutionScaleFactor() { return fAngRes_ScalingFactor; }
        double getArrayCentre_X() { return fArrayCentre_X; }
        double getArrayCentre_Y() { return fArrayCentre_Y; }
	int    getDirectionCutSelector() { return fDirectionCutSelector; }
	int    getGammaHadronCutSelector() { return fGammaHadronCutSelector; }
        double getMeanDistance() { return fMeanDistance; }
        double getMeanLength() { return fMeanLength; }
        double getMeanWidth() { return fMeanWidth; }

	unsigned int getAngularResolutionContainmentRadius() { return fAngResContainmentProbability; }
        double getProbabilityCut_Selector( unsigned int iID = 0 ) { if( iID < fProbabilityCut_NSelectors ) return fProbabilityCut_SelectionCut[iID]; else return -1; }
        double getProbabilityCutAlpha(bool fIsOn);
        double getTheta2Cut_min( double e = 0.1 ) { if( e > 0. ) return fArrayTheta2_min; else return 0.; }
        double getTheta2Cut_max( double e );                           // get theta2 max cut (might be energy dependent)    [TeV] energy (linear)
	TGraph* getTheta2Cut_TMVA_max()  { return fTMVABoxCut_Theta2_max; }
	TGraph* getTheta2Cut_IRF_Max() { return fIRFAngRes; }
        void   printCutSummary();
        void   printCutStatistics() { if( fStats ) fStats->printCutStatistics(); }
	void   printDirectionCuts();
        bool   readCuts(string i_cutfilename);
        bool   readCuts(string i_cutfilename, bool iPrint );
        bool   readCuts(string i_cutfilename, int iPrint );
        void   resetCutValues();
	void   resetCutStatistics();
        void   initializeCuts( int irun, string iDir = "" );
        void   setArrayCentre( double iX = 0., double iY = 0. ) { fArrayCentre_X = iX; fArrayCentre_Y = iY; }
        void   setDataDirectory( string id ) { fDataDirectory = id; }
        bool   setDataTree( CData* idata );
	void   setDebug( bool iB = false ) { fDebug = iB; }
        void   setEnergyCuts( double imin, double imax ) { fArrayErec_min = imin; fArrayErec_max = imax; }
	bool   setIRFGraph( TGraphErrors *g );
        void   setNTel( unsigned int itel,  double iX = 0., double iY = 0. ) { fNTel = itel; fArrayCentre_X = iX; fArrayCentre_Y = iY; }
        void   setShowerCoreCuts( double xmin, double xmax, double ymin, double ymax, double iEdge = -1. );
        void   setTheta2Cut( double it2 ) { fArrayTheta2_max = it2; }
	void   terminate();
	bool   useOrbitalPhaseCuts() { return fUseOrbitalPhaseCuts; }

        ClassDef(VGammaHadronCuts,29);
};
#endif
