/*! \file  trainTMVAforGammaHadronSeparation.cpp
    \brief  use TMVA methods for gamma/hadron separation

    \author Gernot Maier
*/

#include "TCut.h"
#include "TFile.h"
#include "TH1D.h"
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

/*
   check if a training variable is constant

   return values:

   -1:  value is variable
   0-N: value is constant (array identifier)

*/
double checkIfVariableIsConstant( VTMVARunData *iRun, TCut iCut, string iVariable, bool iSignal, bool iSplitBlock )
{
   char hname[2000];
   TH1D *h = 0;
   cout << "initializing TMVA variables: checking";
   if( iSignal ) cout << " signal";
   else          cout << " background";
   cout << " variable " << iVariable << " for consistency " << endl;
   vector< TTree* > iTreeVector;
   if( iSignal ) iTreeVector = iRun->fSignalTree;
   else          iTreeVector = iRun->fBackgroundTree;

// add cut on number of telescope (per type) for 
   if( iVariable.find( "NImages_Ttype" ) != string::npos
    || iVariable.find( "EmissionHeightChi2" ) != string::npos )
   {
      sprintf( hname, "%s >=2 ", iVariable.c_str() );
      if( iVariable.find( "NImages_Ttype" ) != string::npos ) sprintf( hname, "NTtype>0 && %s", hname );
      TCut ntCut( hname );
      iCut = iCut && ntCut;
   }

   for( unsigned  int i = 0; i < iTreeVector.size(); i++ )
   {
      h = 0;
      if( iTreeVector[i] )
      {
// fill a histogram with the variable to be checked
	 sprintf( hname, "hXX_%d", i );
         h = new TH1D( hname, "", 100, -1.e5, 1.e5 );
	 Long64_t iNEntriesBlock = 0;
	 if( iSplitBlock ) iNEntriesBlock = iTreeVector[i]->GetEntries() / 2;
	 else              iNEntriesBlock = iTreeVector[i]->GetEntries();
	 iTreeVector[i]->Project( h->GetName(), iVariable.c_str(), iCut, "", iNEntriesBlock );
	 if( h )
	 {
	    if( h->GetRMS() > 1.e-5 )
	    {
	       cout << "\t variable " << iVariable << " ok, RMS: " << h->GetRMS() << ", tree: " << i;
	       cout << ", nbins " << h->GetNbinsX() << ", xmin " << h->GetXaxis()->GetXmin() << ", xmax " << h->GetXaxis()->GetXmax();
	       cout << ", entries " << h->GetEntries();
	       cout << endl;
	       h->Delete();
	       return -9999.;
	    }
	 }
      }
      if( i < iTreeVector.size() - 1 && h ) h->Delete();
   }
// means: variable is in all trees constant
   cout << "\t warning: constant variable  " << iVariable << " in ";
   if( iSignal ) cout << " signal tree";
   else          cout << " background tree";
   if( h ) cout << " (mean " << h->GetMean() << ", RMS " << h->GetRMS() << ", entries " << h->GetEntries() << ")";
   cout << ", checked " << iTreeVector.size() << " trees";
   cout << endl;
   double i_mean = -9999.;
   if( h ) i_mean = h->GetMean();
   if( h ) h->Delete();

   return i_mean;
}

/*!

     train the MVA

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

// quality cuts before filling
   TCut iCut = iRun->fQualityCuts && iRun->fMCxyoffCut && iRun->fEnergyCutData[iEnergyBin]->fEnergyCut;

// adding training variables
   if( iRun->fTrainingVariable.size() != iRun->fTrainingVariableType.size() )
   {
      cout << "train: error: training-variable vectors have different size" << endl;
      return false;
   }

// check split mode
   bool iSplitBlock = false;
   if( iRun->fPrepareTrainingOptions.find( "SplitMode=Block" ) != string::npos )
   {
      cout << "train: use option SplitMode=Block" << endl;
      iSplitBlock = true;
   }

   for( unsigned int i = 0; i < iRun->fTrainingVariable.size(); i++ )
   {
      if( iRun->fTrainingVariable[i].find( "NImages_Ttype" ) != string::npos )
      {
	 for( int j = 0; j < iRun->fNTtype; j++ )
	 {
	    ostringstream iTemp;
	    iTemp << iRun->fTrainingVariable[i] << "[" << j << "]";
	    ostringstream iTempCut;
// require at least 2 image per telescope type
	    iTempCut << iTemp.str() << ">1";
	    TCut iCutCC = iTempCut.str().c_str();
	    double iSignalMean = checkIfVariableIsConstant( iRun, iCut&&iCutCC, iTemp.str(), true, iSplitBlock );
	    double iBckMean    = checkIfVariableIsConstant( iRun, iCut&&iCutCC, iTemp.str(), false, iSplitBlock );
// check if the training variable is constant 
// (checkIfVariableIsConstant returns -9999 if RMS of variable is >0)
	    cout << "\t mean values " << iSignalMean << "\t" << iBckMean << endl;
	    if( ( TMath::Abs( iSignalMean - iBckMean ) > 1.e-6 
	     || TMath::Abs( iSignalMean + 9999. ) < 1.e-2 || TMath::Abs( iBckMean + 9999. ) < 1.e-2 )
	     && iSignalMean !=0 && iBckMean != 0 )
	    {
	       factory->AddVariable( iTemp.str().c_str(), iRun->fTrainingVariableType[i] );
            }
	    else cout << "warning: removed constant variable " << iTemp.str() << " from training" << endl;
	 }
      }
      else
      {
// check if the training variable is constant
	 double iSignalMean = checkIfVariableIsConstant( iRun, iCut, iRun->fTrainingVariable[i].c_str(), true, iSplitBlock );
	 double iBckMean    = checkIfVariableIsConstant( iRun, iCut, iRun->fTrainingVariable[i].c_str(), false, iSplitBlock );
	 cout << "\t mean values " << iSignalMean << "\t" << iBckMean << endl;
	 if( TMath::Abs( iSignalMean - iBckMean ) > 1.e-6 
	  || TMath::Abs( iSignalMean + 9999. ) < 1.e-2 || TMath::Abs( iBckMean + 9999. ) < 1.e-2 )
	 {
	   factory->AddVariable( iRun->fTrainingVariable[i].c_str(), iRun->fTrainingVariableType[i] );
	 }
	 else cout << "warning: removed constant variable " << iRun->fTrainingVariable[i] << " from training" << endl;
      }
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

// read run parameters from configuration file
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


