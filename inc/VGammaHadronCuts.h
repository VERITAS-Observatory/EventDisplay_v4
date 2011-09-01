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
#include "VInstrumentResponseFunctionData.h"
#include "VTMVAEvaluator.h"
#include "VUtilities.h"

#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TTree.h"

#define VANACUTS_PROBSELECTIONCUTS_MAX 1000

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// enum for efficiency counting of the different types of cuts
enum EN_AnaCutsStats { eTot, eMC_XYoff, eXYoff, eStereoQuality, eArrayChi2, eNImages, eMSC_Quality, eErec, eCorePos, eLTrig, eSizeSecondMax, eTelType, eDirection, eIsGamma, eEnergyRec };

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
/*

   class to keep track of efficiency of different cuts

*/
class VGammaHadronCutsStats : public TNamed
{
   private:

   vector< string > fName;

   public:

   vector< unsigned int > fN;

   VGammaHadronCutsStats();
  ~VGammaHadronCutsStats() {}

   void printCutStatistics();
   void reset();

   ClassDef( VGammaHadronCutsStats, 1 );
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class VGammaHadronCuts : public VAnalysisUtilities
{
    private:

        CData *fData;                                       //! transient
        string fDataDirectory;

// cut selector
        int fCutSelector;                            // see description at beginning of VGammaHadronCuts.cpp

// array characteristics
        unsigned int fNTel;
        double       fArrayCentre_X;
        double       fArrayCentre_Y;

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
	string          fTMVAWeightFile;
	unsigned int    fTMVAWeightFileIndex_min;
	unsigned int    fTMVAWeightFileIndex_max;
	double          fTMVASignalEfficiency;
	double          fTMVAProbabilityThreshold;
	string          fTMVAOptimizeSignalEfficiencyParticleNumberFile;
	double          fTMVAOptimizeSignalEfficiencySourceStrengthCU;
	bool            fTMVAIgnoreTheta2Cut;
	TGraph*         fTMVABoxCut_Theta2_max;                


// parameters for energy dependent theta2 cuts
// (implemented for MC only)
        string fFileNameAngRes;
        TFile *fFileAngRes;                                         //!
        string fF1AngResName;
        TF1   *fF1AngRes;
        double fAngRes_ScalingFactor;
        double fAngRes_AbsoluteMinimum;
	double fAngRes_AbsoluteMaximum;
	TGraphErrors *fIRFAngRes;

        bool initAngularResolutionFile();

// cut statistics
        VGammaHadronCutsStats* fStats;

        bool   applyProbabilityCut( int i, bool fIsOn);
	bool   applyTMVACut( int i, bool fIsOn );
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
        double fMaxone_min;
        double fMaxtwo_min;
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

        vector <double> fProbabilityCutRangeLower;
        vector <double> fProbabilityCutRangeUpper;
        	  
        vector< VNTelTypeCut* > fNTelTypeCut;

        bool   bMCCuts;
        double fArrayxyoff_MC_min;
        double fArrayxyoff_MC_max;

        double fProbabilityCut;


        VGammaHadronCuts();
       ~VGammaHadronCuts();

        bool   applyDirectionCuts( unsigned int fEnergyReconstructionMethod = 0, bool bCount = false, double x0 = -99999., double y0 = -99999. );
        bool   applyEnergyReconstructionQualityCuts();
        bool   applyEnergyReconstructionQualityCuts( unsigned int iMethod, bool bCount = false );
        bool   applyInsideFiducialAreaCut( bool bCount = false );
        bool   applyMCXYoffCut( double x, double y, bool bCount = false );
	bool   applyMeanReducedScaledStereoShapeCuts();
	bool   applyMeanStereoShapeCuts();
        bool   applyMeanScaledStereoShapeCuts();
        bool   applyShowerCoreCuts( bool iMC = false );
        bool   applyStereoQualityCuts( unsigned int iMethod = 0, bool bCount = false, int iEntry = 0, bool fIsOn = false );
        bool   applyStereoShapeCuts();
        bool   applyTelTypeTest( bool bCount = false );

        bool   isGamma() { return isGamma( 0 ); }
        bool   isGamma( int i, bool bCount = false, bool fIsOn = true);

        bool   isMCCuts() { return bMCCuts; }

        void   newEvent();


        TF1*   getAngularResolutionFunction() { return fF1AngRes; }
        double getAngularResolutionAbsoluteMinimum() { return fAngRes_AbsoluteMinimum; }
        double getAngularResolutionScaleFactor() { return fAngRes_ScalingFactor; }
        double getArrayCentre_X() { return fArrayCentre_X; }
        double getArrayCentre_Y() { return fArrayCentre_Y; }
        double getMeanDistance() { return fMeanDistance; }
        double getMeanLength() { return fMeanLength; }
        double getMeanWidth() { return fMeanWidth; }

        double getProbabilityCut_Selector( unsigned int iID = 0 ) { if( iID < fProbabilityCut_NSelectors ) return fProbabilityCut_SelectionCut[iID]; else return -1; }
        double getProbabilityCutAlpha(bool fIsOn);
        double getTheta2Cut_min( double e = 0.1 ) { if( e > 0. ) return fArrayTheta2_min; else return 0.; }
        double getTheta2Cut_max( double e );                           // get theta2 max cut (might be energy dependent)    [TeV] energy (linear)
	TGraph* getTheta2Cut_TMVA_max()  { return fTMVABoxCut_Theta2_max; }
        void   printCutSummary();
        void   printCutStatistics() { if( fStats ) fStats->printCutStatistics(); }
        bool   readCuts(string i_cutfilename);
        bool   readCuts(string i_cutfilename, bool iPrint );
        bool   readCuts(string i_cutfilename, int iPrint );
        void   resetCutValues();
        void   selectCuts( int iC, int irun, string iDir = "" );
        void   setArrayCentre( double iX = 0., double iY = 0. ) { fArrayCentre_X = iX; fArrayCentre_Y = iY; }
        void   setDataDirectory( string id ) { fDataDirectory = id; }
        bool   setDataTree( CData* idata );
        void   setEnergyCuts( double imin, double imax ) { fArrayErec_min = imin; fArrayErec_max = imax; }
        void   setNTel( unsigned int itel,  double iX = 0., double iY = 0. ) { fNTel = itel; fArrayCentre_X = iX; fArrayCentre_Y = iY; }
        void   setShowerCoreCuts( double xmin, double xmax, double ymin, double ymax, double iEdge = -1. );
        void   setTheta2Cut( double it2 ) { fArrayTheta2_max = it2; }

        ClassDef(VGammaHadronCuts,14);
};
#endif
