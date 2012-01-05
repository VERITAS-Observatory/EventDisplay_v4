/* \file trainTMVAforAngularReconstruction
   \brief use TMVA methods for modified disp method

   data is first filled into a tree with all necessary parameters

*/

#include "TChain.h"
#include "TCut.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"

#include "TMVA/Config.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "Ctelconfig.h"
#include "Cshowerpars.h"
#include "CtparsShort.h"
#include "VCameraRead.h"
#include "VDetectorTree.h"
#include "VGlobalRunParameter.h"

using namespace std;

// one tree per telescope type
map< ULong64_t, TTree* > fMapOfTrainingTree;

float isRMSZero( TTree *t, string iVar )
{
   if( !t || iVar.size() == 0 ) return -1.;

   float f = 0.;
   float i_sum = 0.;
   float i_n = 0.;
   t->SetBranchAddress( iVar.c_str(), &f );

// check first 1000 entries;
   int nentries = 1000;
   if( t->GetEntries() < nentries ) nentries = t->GetEntries();

   for( int i = 0; i < nentries; i++ )
   {
     t->GetEntry( i );

     if( f > 0. )
     {
        i_sum += f;
	i_n++;
     }
   }

   if( i_n > 0. ) return i_sum / i_n;

   return -1.;
}

/*

    train TVMA method and write results into the corresponding directory

    one MVA per telescope type

*/
bool trainTMVA( string iOutputDir, string iOutputName )
{

    map< ULong64_t, TTree* >::iterator fMapOfTrainingTree_iter;
    for( fMapOfTrainingTree_iter = fMapOfTrainingTree.begin(); fMapOfTrainingTree_iter != fMapOfTrainingTree.end(); fMapOfTrainingTree_iter++ )
    {
       if( !fMapOfTrainingTree_iter->second ) continue;

// output file name
       ostringstream iFileName;
       iFileName << iOutputDir << "/" << iOutputName << "_" << fMapOfTrainingTree_iter->first << ".tmva.root";
       TFile i_tmva( iFileName.str().c_str(), "RECREATE" );
       if( i_tmva.IsZombie() )
       {
          cout << "error while creating tmva root file: " << iFileName.str() << endl;
	  return false;
       } 

// set output directory
       gSystem->mkdir( iOutputDir.c_str() );
       TString iOutputDirectory( iOutputDir.c_str() );
       gSystem->ExpandPathName( iOutputDirectory );
       (TMVA::gConfig().GetIONames()).fWeightFileDir = iOutputDirectory;

// tmva regression
       TMVA::Factory *factory = new TMVA::Factory( "BDTDisp", &i_tmva, "V" );
       factory->AddVariable( "width", 'F' );
       factory->AddVariable( "length", 'F' );
       factory->AddVariable( "size", 'F' );
       factory->AddVariable( "asym", 'F' );
// add pedvar only if different from 0 (for CTA: always zero)
/*       if( isRMSZero( fMapOfTrainingTree_iter->second, "meanPedvar_Image" ) > 1.e-2 )
       {
          factory->AddVariable( "meanPedvar_Image", 'F' );
       } */
       factory->AddVariable( "tgrad_x", 'F' );

       factory->AddTarget( "disp", 'F' );
//       factory->AddTarget( "dispPhi", 'F' );

       factory->AddRegressionTree( fMapOfTrainingTree_iter->second, 1. );

// quality cuts
       TCut fQualityCut = "size>0.&&ntubes>3.&&length>0.";

       factory->PrepareTrainingAndTestTree( fQualityCut, "nTrain_Regression=0:nTest_Regression=0:SplitMode=Random:NormMode=NumEvents:!V" );


       ostringstream iMVAName;
       iMVAName << "BDT_" << fMapOfTrainingTree_iter->first;
       cout << iMVAName.str() << endl;
       factory->BookMethod( TMVA::Types::kBDT, iMVAName.str().c_str(), "" );

       factory->TrainAllMethodsForRegression();

       factory->TestAllMethods();

       factory->EvaluateAllMethods();

       i_tmva.Close();

       delete factory;
    }

   return true;
}

/*
   write the training file used for the TMVA training
   (simply a tree with all the necessary variables; one tree per telescope type)

*/
bool writeTrainingFile( string iInputFile, string iOutputDir, string iOutputName )
{

// telescope configuration
    TChain i_telChain( "telconfig" );
    i_telChain.Add( iInputFile.c_str(), 0 );

    Ctelconfig i_tel( &i_telChain );
    i_tel.GetEntry( 0 );
    unsigned int i_ntel = i_tel.NTel;

// training trees (one per telescope type)

    float cen_x = -1.;
    float cen_y = -1.;
    float sinphi = -1.;
    float cosphi = -1.;
    float size = -1.;
    float ntubes = -1.;
    float loss = -1.;
    float asym = -1.;
    float width = -1.;
    float length = -1.;
    float MCe0 = -1.;
    float MCxoff = -1.;
    float MCyoff = -1.;
    float MCxcore = -1.;
    float MCycore = -1.;
    float MCaz = -1.;
    float disp = -1.;
    float dispPhi = -1.;
    float dist = -1.;
    float tgrad_x = -1.;
    float meanPedvar_Image = -1.;
    float ze = -1.;
    float az = -1.;

    fMapOfTrainingTree.clear();
    cout << "total number of telescopes: " << i_ntel << endl;
    for( unsigned int i = 0; i < i_ntel; i++ )
    {
       i_tel.GetEntry( i );

       if( fMapOfTrainingTree.find( i_tel.TelType ) == fMapOfTrainingTree.end() ) 
       {
	  ostringstream iTreeName;
	  iTreeName << "dispTree_" << i_tel.TelType;
	  ostringstream iTreeTitle;
	  iTreeTitle << "training tree for modified disp method (telescope type " << i_tel.TelType << ")";
          fMapOfTrainingTree[i_tel.TelType] = new TTree( iTreeName.str().c_str(), iTreeTitle.str().c_str() );

	  fMapOfTrainingTree[i_tel.TelType]->Branch( "cen_x", &cen_x, "cen_x/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "cen_y", &cen_y, "cen_y/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "sinphi", &sinphi, "sinphi/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "cosphi", &cosphi, "cosphi/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "size", &size, "size/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "ntubes", &ntubes, "ntubes/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "loss", &loss, "loss/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "asym", &asym, "asym/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "width", &width, "width/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "length", &length, "length/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "dist", &dist, "dist/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "tgrad_x", &tgrad_x, "tgrad_x/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "meanPedvar_Image", &meanPedvar_Image, "meanPedvar_Image/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "MCe0", &MCe0, "MCe0/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "MCxoff", &MCxoff, "MCxoff/F " );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "MCyoff", &MCyoff, "MCyoff/F " );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "MCxcore", &MCxcore, "MCxcore/F " );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "MCycore", &MCycore, "MCycore/F " );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "MCaz", &MCaz, "MCaz/F " );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "Ze", &ze, "Ze/F " );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "Az", &az, "Az/F " );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "disp", &disp, "disp/F" );
	  fMapOfTrainingTree[i_tel.TelType]->Branch( "dispPhi", &dispPhi, "dispPhi/F" );

       }
    }
    cout << "filling training trees for " << fMapOfTrainingTree.size() << " telescope types" << endl;

// get showerpars tree
    TChain i_showerparsTree( "showerpars" );
    i_showerparsTree.Add( iInputFile.c_str(), 0 );
    Cshowerpars i_showerpars( &i_showerparsTree, true, 6, true );

// loop over all telescopes and fill training trees
    for( unsigned int i = 0; i < i_ntel; i++ )
    {
       i_tel.GetEntry( i );

       ostringstream iTreeName;
       iTreeName << "Tel_" << i+1 << "/tpars";
       TChain i_tparsTree( iTreeName.str().c_str() );
       i_tparsTree.Add( iInputFile.c_str(), 0 );
       CtparsShort i_tpars( &i_tparsTree, true, 6, true );

       cout << "\t found tree " << iTreeName.str() << ", entries: " << i_showerpars.fChain->GetEntries() << ", ";
       cout << i_tpars.fChain->GetEntries() << endl;

       for( int n = 0; n < i_showerpars.fChain->GetEntries(); n++ )
       {
          i_showerpars.GetEntry( n );
	  i_tpars.GetEntry( n );

// quality cuts
          if( i_tpars.ntubes < 4 || i_tpars.size < 0 || i_tpars.loss > 0.2 ) continue;

	  cen_x = i_tpars.cen_x;
	  cen_y = i_tpars.cen_y;
	  sinphi = i_tpars.sinphi;
	  cosphi = i_tpars.cosphi;
	  size = i_tpars.size;
	  ntubes = i_tpars.ntubes;
	  loss = i_tpars.loss;
	  asym = i_tpars.asymmetry;
	  width = i_tpars.width;
	  length = i_tpars.length;
	  dist = i_tpars.dist;
	  tgrad_x = i_tpars.tgrad_x;
	  meanPedvar_Image = i_tpars.meanPedvar_Image;
	  ze = 90.-i_showerpars.TelElevation[i];
	  az = i_showerpars.TelAzimuth[i];
	  MCe0 = i_showerpars.MCe0;
	  MCxoff = i_showerpars.MCxoff;
	  MCyoff = i_showerpars.MCyoff;
	  MCxcore = i_showerpars.MCxcore;
	  MCycore = i_showerpars.MCycore;
	  MCaz = i_showerpars.MCaz;

// calculate disp (observe sign convention for MCyoff)
	  disp = sqrt( (cen_y+MCyoff)*(cen_y+MCyoff) + (cen_x-MCxoff)*(cen_x-MCxoff) );
	  dispPhi = TMath::ATan2( sinphi, cosphi)-TMath::ATan2( cen_y+MCyoff, cen_x-MCxoff );

	  if( fMapOfTrainingTree.find( i_tel.TelType ) != fMapOfTrainingTree.end() ) 
	  {
	     fMapOfTrainingTree[i_tel.TelType]->Fill();
          }
       }
   }


   return true;
}


int main( int argc, char *argv[] )
{

    cout << endl;
    if( argc < 4 )
    {
        cout << "./trainTMVAforAngularReconstruction <input eventdisplay file (MC)> <output directory> <training file name>" << endl;
	cout << endl;
	exit( 0 );
    }
    string fInputFile = argv[1];
    string fOutputDir = argv[2];
    string fOutputName = argv[3];

// output file
    ostringstream iFileName;
    iFileName << fOutputDir << "/" << fOutputName << ".root";
    TFile iO( iFileName.str().c_str(), "RECREATE" );
    if( iO.IsZombie() )
    {
       cout << "Error creating output file: " << iFileName.str() << endl;
       return false;
    }
// fill training file
    if( !writeTrainingFile( fInputFile, fOutputDir, fOutputName ) )
    {
       cout << "error writing training file " << endl;
       exit( -1 );
    } 
// write training tree to output file
    map< ULong64_t, TTree* >::iterator fMapOfTrainingTree_iter;
    for( fMapOfTrainingTree_iter = fMapOfTrainingTree.begin(); fMapOfTrainingTree_iter != fMapOfTrainingTree.end(); fMapOfTrainingTree_iter++ )
    {
       if( fMapOfTrainingTree_iter->second ) fMapOfTrainingTree_iter->second->Write();
    }

// train TMVA
    trainTMVA( fOutputDir, fOutputName );

// close output file
    iO.Close();

}
