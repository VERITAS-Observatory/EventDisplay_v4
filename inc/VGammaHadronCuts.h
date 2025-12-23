//! VGammaHadronCuts: class for parameter cut definitions

#ifndef VGammaHadronCuts_H
#define VGammaHadronCuts_H

#include <algorithm>
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
// analysis types
////////////////////////////////////////////////////////////////////////////////
enum E_AnalysisType { GEO = 0, MVAAnalysis = 1 };

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

        ClassDef( VNTelTypeCut, 2 );
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

        ClassDef( VGammaHadronCutsStats, 3 );
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class VGammaHadronCuts : public VAnalysisUtilities
{
    private:

        bool   fDebug;                               // lots of debug output

        CData* fData;                                       //! transient
        string fDataDirectory;
        string fInstrumentEpoch;

        // cut selector
        int fGammaHadronCutSelector;                            // see description at beginning of VGammaHadronCuts.cpp
        int fDirectionCutSelector;
        E_AnalysisType fAnalysisType;

        // array characteristics (number of telescopes, centre of array)
        unsigned int fNTel;
        double       fArrayCentre_X;
        double       fArrayCentre_Y;

        // number of possible telescope combinations
        unsigned int fNLTrigs;

        // telescope used in analysis (optional)
        vector< unsigned int > fTelToAnalyze;

        // values calculated from shower/image parameter
        double fMeanImageDistance;
        double fMeanImageLength;
        double fMeanImageWidth;

        // event by event cuts (read in from an additional friend tree, used by random forest analysis, pulsar analysis, etc)
        TFile* fProbabilityCut_File;                  //!
        TTree* fProbabilityCut_Tree;                  //!
        int fProbabilityCut_QualityFlag;              // quality flag for probability cut 0: cut estimation failed; >0 successful probability estimation
        unsigned int fProbabilityCut_NSelectors;      // number of elements in fProbabilityCut_SelectionCut[]
        unsigned int fProbabilityCut_ProbID;          // array element to be used from fProbabilityCut_SelectionCut[]
        double fProbabilityCut_SelectionCut[VANACUTS_PROBSELECTIONCUTS_MAX];    // selection cut

        //////////////////////////
        // TMVA evaluator
        VTMVAEvaluator* fTMVAEvaluator;                             //!
        string          fTMVA_MVAMethod;
        string          fTMVAWeightFile;
        unsigned int    fTMVAWeightFileIndex_Emin;
        unsigned int    fTMVAWeightFileIndex_Emax;
        unsigned int    fTMVAWeightFileIndex_Zmin;
        unsigned int    fTMVAWeightFileIndex_Zmax;
        map< unsigned int, double > fTMVASignalEfficiency;
        map< unsigned int, double > fTMVA_MVACut;
        double          fTMVAProbabilityThreshold;
        string          fTMVAOptimizeSignalEfficiencyParticleNumberFile;
        double          fTMVAParticleNumberFile_Conversion_Rate_to_seconds;
        double          fTMVAOptimizeSignalEfficiencySignificance_Min;
        double          fTMVAOptimizeSignalEfficiencySignalEvents_Min;
        double          fTMVAOptimizeSignalEfficiencyObservationTime_h;
        double          fTMVAFixedSignalEfficiencyMax;
        double          fTMVAMinSourceStrength;
        double          fTMVAFixedThetaCutMin;
        double          fTMVA_EvaluationResult;
        VTMVAEvaluatorResults* fTMVAEvaluatorResults;
        // TMVA results read from the

        // orbital phase analysis
        TFile* fPhaseCut_File;                                      //!
        TTree* fPhaseCut_Tree;                                      //!
        double fOrbitalPhase;

        // parameters for energy dependent theta2 cuts
        // (implemented for MC only)
        string fFileNameAngRes;
        TFile* fFileAngRes;                                         //!
        string fF1AngResName;
        TF1*   fF1AngRes;
        double       fAngRes_ScalingFactor;
        double       fAngRes_AbsoluteMinimum;
        double       fAngRes_AbsoluteMaximum;
        unsigned int fAngResContainmentProbability;

        //////////////////////////
        // energy dependent cuts
        map< string, TGraph* > fEnergyDependentCut;

        // cut statistics
        VGammaHadronCutsStatistics* fStats;                       //!

        bool   applyProbabilityCut( int i, bool fIsOn );
        double getEnergyDependentCut( double energy_TeV, TGraph* iG, bool bUseEvalue = true, bool bMaxCut = true );
        TGraph* getEnergyDependentCut( string iCutName );
        bool   getEnergyDependentCutFromFile( string iFileName, string iVariable );
        bool   initAngularResolutionFile();
        bool   initPhaseCuts( int irun );
        bool   initPhaseCuts( string iDir );
        bool   initProbabilityCuts( int irun );
        bool   initProbabilityCuts( string iDir );
        bool   initTMVAEvaluator( string iTMVAFile, unsigned int iTMVAWeightFileIndex_Emin, unsigned int iTMVAWeightFileIndex_Emax, unsigned int iTMVAWeightFileIndex_Zmin, unsigned int iTMVAWeightFileIndex_Zmax );
        string getTelToAnalyzeString();


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////

    public:
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
        double fCut_DispIntersectDiff_min;
        double fCut_DispIntersectDiff_max;
        int    fCut_DispIntersectSuccess;
        double fProbabilityCut;
        vector <double> fProbabilityCutRangeLower;
        vector <double> fProbabilityCutRangeUpper;

        vector< VNTelTypeCut* > fNTelTypeCut;

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
        bool   applyPhaseCut( int i );
        bool   applyStereoQualityCuts( unsigned int iEnergyReconstructionMethod = 0, bool bCount = false, int iEntry = 0, bool fIsOn = false );
        bool   applyStereoShapeCuts();
        bool   applyTMVACut( int i );

        TF1*   getAngularResolutionFunction()
        {
            return fF1AngRes;
        }
        double getAngularResolutionAbsoluteMinimum()
        {
            return fAngRes_AbsoluteMinimum;
        }
        double getAngularResolutionAbsoluteMaximum()
        {
            return fAngRes_AbsoluteMaximum;
        }
        double getAngularResolutionScaleFactor()
        {
            return fAngRes_ScalingFactor;
        }
        double getArrayCentre_X()
        {
            return fArrayCentre_X;
        }
        double getArrayCentre_Y()
        {
            return fArrayCentre_Y;
        }
        double getReconstructedEnergy( unsigned int iEnergyReconstructionMethod = 0 );
        double getReconstructedEnergyChi2( unsigned int iEnergyReconstructionMethod = 0 );
        double getReconstructedEnergydE( unsigned int iEnergyReconstructionMethod = 0. );
        double getReconstructedXoff();
        double getReconstructedYoff();
        double getReconstructedXcore();
        double getReconstructedYcore();
        int    getDirectionCutSelector()
        {
            return fDirectionCutSelector;
        }
        int    getGammaHadronCutSelector()
        {
            return fGammaHadronCutSelector;
        }
        double getMeanImageDistance()
        {
            return fMeanImageDistance;
        }
        double getMeanImageLength()
        {
            return fMeanImageLength;
        }
        double getMeanImageWidth()
        {
            return fMeanImageWidth;
        }
        unsigned int getAngularResolutionContainmentRadius()
        {
            return fAngResContainmentProbability;
        }
        double getProbabilityCut_Selector( unsigned int iID = 0 )
        {
            if( iID < fProbabilityCut_NSelectors )
            {
                return fProbabilityCut_SelectionCut[iID];
            }
            else
            {
                return -1;
            }
        }
        double getProbabilityCutAlpha( bool fIsOn );
        double getTheta2Cut_min( double e = 0.1 )
        {
            if( e > 0. )
            {
                return fCut_Theta2_min;
            }
            else
            {
                return 0.;
            }
        }
        double getTheta2Cut_max()
        {
            return fCut_Theta2_max;
        }
        double getTheta2Cut_max( double e );                           // get theta2 max cut (might be energy dependent)    [TeV] energy (linear)
        TGraph* getTheta2Cut_IRF_Max()
        {
            return getEnergyDependentCut( "IRFAngRes" );
        }
        double getTMVA_EvaluationResult()
        {
            return fTMVA_EvaluationResult;
        }
        VTMVAEvaluatorResults* getTMVAEvaluatorResults()
        {
            return fTMVAEvaluatorResults;
        }
        void   initialize();
        bool   isGamma( int i = 0, bool bCount = false, bool fIsOn = true );
        bool   isMCCuts()
        {
            return bMCCuts;
        }
        void   newEvent( bool iFillStats = true );
        void   printCutSummary();
        void   printCutStatistics()
        {
            if( fStats )
            {
                fStats->printCutStatistics();
            }
        }
        void   printDirectionCuts();
        void   printEnergyDependentCuts();
        void   printSignalEfficiency();
        void   printTMVA_MVACut();
        bool   readCuts( string i_cutfilename, int iPrint = 1 );
        void   resetCutValues();
        void   resetCutStatistics();
        void   initializeCuts( int irun = -1, string iDir = "" );
        void   setArrayCentre( double iX = 0., double iY = 0. )
        {
            fArrayCentre_X = iX;
            fArrayCentre_Y = iY;
        }
        void   setDataDirectory( string id )
        {
            fDataDirectory = id;
        }
        bool   setDataTree( CData* idata );
        void   setDebug( bool iB = false )
        {
            fDebug = iB;
        }
        void   setEnergyCuts( double imin, double imax )
        {
            fCut_Erec_min = imin;
            fCut_Erec_max = imax;
        }
        void   setInstrumentEpoch( string iEpoch )
        {
            fInstrumentEpoch = iEpoch;
        }
        bool   setIRFGraph( TGraphErrors* g );
        void   setNTel( unsigned int itel,  double iX = 0., double iY = 0. )
        {
            fNTel = itel;
            fArrayCentre_X = iX;
            fArrayCentre_Y = iY;
        }
        void   setTelToAnalyze( vector< unsigned int > iTelToAnalyze )
        {
            fTelToAnalyze = iTelToAnalyze;
        }
        void   setTheta2Cut( double it2 )
        {
            fCut_Theta2_max = it2;
        }
        void   terminate();
        bool   useTMVACuts()
        {
            return ( fAnalysisType == MVAAnalysis );
        }
        bool   useOrbitalPhaseCuts()
        {
            return fUseOrbitalPhaseCuts;
        }

        ClassDef( VGammaHadronCuts, 59 );
};
#endif
