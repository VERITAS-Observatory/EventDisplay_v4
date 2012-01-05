/*! \file  trainTMVAforGammaHadronSeparation.cpp
    \brief  use TMVA methods for gamma/hadron separation

    $Rev$
    $Date$
    $Author$

*/

#include "TCut.h"
#include "TFile.h"
#include "TMath.h"
#include "TSystem.h"
#include "TTree.h"

#include "TMVA/Config.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "VTMVARunData.h"

using namespace std;

/*!
     run the optimization
*/
bool train( VTMVARunData *iRun, unsigned int iEnergyBin )
{
// sanity checks
    if( !iRun ) return false;
    if( iRun->fEnergyCutData.size() < iEnergyBin || iRun->fOutputFile.size() < iEnergyBin )
    {
        cout << "error in train: energy bin out of range " << iEnergyBin;
	cout << "\t" << iRun->fEnergyCutData.size() << "\t" << iRun->fOutputFile.size() << endl;
        return false;
    }


    TMVA::Tools::Instance();

// set output directory
    gSystem->mkdir( iRun->fOutputDirectoryName.c_str() );
    TString iOutputDirectory( iRun->fOutputDirectoryName.c_str() );
    gSystem->ExpandPathName( iOutputDirectory );
    (TMVA::gConfig().GetIONames()).fWeightFileDir = iOutputDirectory;

//////////////////////////////////////////
// defining training class
    TMVA::Factory *factory = new TMVA::Factory( iRun->fOutputFileName.c_str(), iRun->fOutputFile[iEnergyBin], "V" );

// adding signal and background trees
    for( unsigned int i = 0; i < iRun->fSignalTree.size(); i++ )
    {
       factory->AddSignalTree( iRun->fSignalTree[i], iRun->fSignalWeight );
    }
    for( unsigned int i = 0; i < iRun->fBackgroundTree.size(); i++ )
    {
       factory->AddBackgroundTree( iRun->fBackgroundTree[i], iRun->fBackgroundWeight );
    }

// adding training variables
   if( iRun->fTrainingVariable.size() != iRun->fTrainingVariableType.size() )
   {
      cout << "train: error: training-variable vectors have different size" << endl;
      return false;
   }
   for( unsigned int i = 0; i < iRun->fTrainingVariable.size(); i++ )
   {
      factory->AddVariable( iRun->fTrainingVariable[i].c_str(), iRun->fTrainingVariableType[i] );
   }
// adding spectator variables
   for( unsigned int i = 0; i < iRun->fSpectatorVariable.size(); i++ )
   {
      factory->AddSpectator( iRun->fSpectatorVariable[i].c_str() );
   }

// weight expression
//   factory->SetWeightExpression( "InputWeight" );


//////////////////////////////////////////
// prepare training events
// nTrain Signal=5000:nTrain Background=5000: nTest Signal=4000:nTest Background=5000
   factory->PrepareTrainingAndTestTree( iRun->fQualityCuts && iRun->fMCxyoffCut && 
                                        iRun->fEnergyCutData[iEnergyBin]->fEnergyCut, 
					iRun->fPrepareTrainingOptions );
      
//////////////////////////////////////////
// book all methods
   char hname[6000];
   char htitle[6000];

   for( unsigned int i = 0; i < iRun->fMVAMethod.size(); i++ )
   {
// BOOSTED DECISION TREES
       if( iRun->fMVAMethod[i] == "BDT" )
       {
	  sprintf( htitle, "BDT_%d", iEnergyBin );
	  if( i < iRun->fMVAMethod_Options.size() )
	  {
	     factory->BookMethod( TMVA::Types::kBDT, htitle, iRun->fMVAMethod_Options[i].c_str() );
          }
	  else
	  {
	     factory->BookMethod( TMVA::Types::kBDT, htitle );
          }
       }
// BOX CUTS
       if( iRun->fMVAMethod[i] == "BOXCUTS" )
       {
          if( i < iRun->fMVAMethod_Options.size() )  sprintf( hname, "%s", iRun->fMVAMethod_Options[i].c_str() );

	  for( unsigned int i = 0; i < iRun->fTrainingVariable_CutRangeMin.size(); i++ ) 
	  {
	    sprintf( hname, "%s:CutRangeMin[%d]=%f", hname, i, iRun->fTrainingVariable_CutRangeMin[i] );
	  }
	  for( unsigned int i = 0; i < iRun->fTrainingVariable_CutRangeMax.size(); i++ )
	  {
	    sprintf( hname, "%s:CutRangeMax[%d]=%f", hname, i, iRun->fTrainingVariable_CutRangeMax[i] );
	  }
	  for( unsigned int i = 0; i < iRun->fTrainingVariable_VarProp.size(); i++ )
	  {
	    sprintf( hname, "%s:VarProp[%d]=%s", hname, i, iRun->fTrainingVariable_VarProp[i].c_str() );
	  }
	  sprintf( htitle, "BOXCUTS_%d", iEnergyBin );
          factory->BookMethod( TMVA::Types::kCuts, htitle, hname );
       }
   }


//////////////////////////////////////////
// start training

   factory->TrainAllMethods();

//////////////////////////////////////////
// evaluate results

   factory->TestAllMethods();

   factory->EvaluateAllMethods();

   return true;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] )
{
    if( argc != 2 )
    {
	cout << endl;
        cout << "trainTMVAforGammaHadronSeparation <configuration file>" << endl;
	cout << endl;
        exit( 0 );
    }

    cout << endl;
    cout << "trainTMVAforGammaHadronSeparation " << endl;
    cout << "=================================" << endl;
    cout << endl;

// data object
   VTMVARunData *fData = new VTMVARunData();
   fData->fName = "OO";

// read run parameters
   if( !fData->readConfigurationFile( argv[1] ) )
   {
      cout << "error opening run parameter file (";
      cout << argv[1];
      cout << ")" << endl;
      exit( -1 );
   }
   fData->print();

// read and prepare data files
    if( !fData->openDataFiles() )
    {
       cout << "error opening data files" << endl;
       exit( -1 );
    }

// optimize cuts
    cout << "Total number of energy bins: " << fData->fEnergyCutData.size() << endl;
    cout << "================================" << endl << endl;
    for( unsigned int i = 0; i < fData->fEnergyCutData.size(); i++ )
    {
       if( fData->fEnergyCutData[i]->fEnergyCut )
       {
          cout << "Training energy bin " << fData->fEnergyCutData[i]->fEnergyCut << endl;
	  cout << "===================================================================================" << endl;
	  cout << endl;
       }
       train( fData, i );
    }

    return 0;
}

