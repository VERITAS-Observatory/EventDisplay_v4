//! VTMVARunData run parameter data class for TMVA optimization

#ifndef VTMVARunData_H
#define VTMVARunData_H

#include "TChain.h"
#include "TCut.h"
#include "TFile.h"
#include "TEntryList.h"
#include "TMath.h"
#include "TNamed.h"
#include "TSystem.h"
#include "TTree.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "VTableLookupRunParameter.h"
#include "VTMVARunDataEnergyCut.h"
#include "VTMVARunDataZenithCut.h"
#include "VUtilities.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
// data class with all signal/background files names and run parameters

class VTMVARunData : public TNamed
{
    private:

        bool              fDebug;

        bool         fillEnergyCutData(
            vector< double > iEnergyCut_Log10TeV_min, vector< double > iEnergyCut_Log10TeV_max );
        unsigned int getTrainOptionValue( string iVarName, unsigned int i_default );

    public:

        string            fName;
        string            fRunOption;

        // run type
        bool fTrainGammaHadronSeparation;
        bool fTrainReconstructionQuality;

        // output file
        string            fOutputFileName;
        string            fOutputDirectoryName;
        vector< vector< TFile* > >  fOutputFile;
        string            fSelectedEventFileName;

        // training data
        double            fSignalWeight;
        vector< string >  fSignalFileName;
        vector< TChain* > fSignalTree;
        double            fBackgroundWeight;
        vector< string >  fBackgroundFileName;
        vector< TChain* > fBackgroundTree;
        unsigned int      fnTrain_Signal;
        unsigned int      fnTrain_Background;

        // list of training variables
        vector< string >  fTrainingVariable;
        vector< char >    fTrainingVariableType;
        vector< float >   fTrainingVariable_CutRangeMin;
        vector< float >   fTrainingVariable_CutRangeMax;
        vector< string >  fTrainingVariable_VarProp;

        // spectator variables
        vector< string > fSpectatorVariable;

        // quality and energy and zenith cuts
        unsigned int      fMinSignalEvents;
        unsigned int      fMinBackgroundEvents;
        TCut              fQualityCuts;
        TCut              fQualityCutsBkg;
        TCut              fMCxyoffCut;
        bool              fMCxyoffCutSignalOnly;
        string            fPrepareTrainingOptions;
        vector< VTMVARunDataEnergyCut* > fEnergyCutData;
        vector< VTMVARunDataZenithCut* > fZenithCutData;

        // analysis variables
        int               fNTtype;

        // MVA methods
        vector< string >  fMVAMethod;
        vector< string >  fMVAMethod_Options;

        // reconstruction quality target
        string            fReconstructionQualityTarget;
        string            fReconstructionQualityTargetName;

        VTMVARunData();
        ~VTMVARunData() {}
        void print();
        VTableLookupRunParameter* getTLRunParameter();
        bool readConfigurationFile( char* );
        bool openDataFiles();
        void setDebug( bool iB = true )
        {
            fDebug = iB;
        }
        void setName( string iN )
        {
            fName = iN;
        }
        void shuffleFileVectors();
        void updateTrainingEvents( string iVarName, unsigned int iNEvents );

        ClassDef( VTMVARunData, 11 );
};

#endif
