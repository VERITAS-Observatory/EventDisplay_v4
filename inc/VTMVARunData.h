//! VTMVARunData run parameter data class for TMVA optimization
// Revision $Id: VTMVARunData.h,v 1.1.2.5 2011/04/11 16:09:05 gmaier Exp $

#ifndef VTMVARunData_H
#define VTMVARunData_H

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
#include <sstream>
#include <string>
#include <vector>

#include "VTMVARunDataEnergyCut.h"
#include "VUtilities.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
// data class with all signal/background files names and run parameters

class VTMVARunData : public TNamed
{
    private:

    bool              fDebug;

    public:

    string            fName;

// output file
    string            fOutputFileName;
    string            fOutputDirectoryName;
    vector< TFile* >  fOutputFile;             

// training data
    double            fSignalWeight;
    vector< string >  fSignalFileName;
    vector< TFile* >  fSignalFile;            
    vector< TTree* >  fSignalTree;
    double            fBackgroundWeight;
    vector< string >  fBackgroundFileName;
    vector< TFile* >  fBackgroundFile;
    vector< TTree* >  fBackgroundTree;

// list of training variables
    vector< string >  fTrainingVariable;
    vector< char >    fTrainingVariableType;
    vector< float >   fTrainingVariable_CutRangeMin;
    vector< float >   fTrainingVariable_CutRangeMax;
    vector< string >  fTrainingVariable_VarProp;

// spectator variables
    vector< string > fSpectatorVariable;

// quality and energy cuts
    unsigned int      fMinSignalEvents;
    unsigned int      fMinBackgroundEvents;
    TCut              fQualityCuts;
    TCut              fMCxyoffCut;
    string            fPrepareTrainingOptions;
    vector< VTMVARunDataEnergyCut* > fEnergyCutData;

// analysis variables
    int               fNTtype;

// MVA methods
    vector< string >  fMVAMethod;
    vector< string >  fMVAMethod_Options;

    VTMVARunData();
   ~VTMVARunData() {}
    void print();
    bool readConfigurationFile( char* );
    bool openDataFiles();
    void setDebug( bool iB = true ) { fDebug = iB; }
    void setName( string iN ) { fName = iN; }

    ClassDef(VTMVARunData, 6 );
};

#endif
