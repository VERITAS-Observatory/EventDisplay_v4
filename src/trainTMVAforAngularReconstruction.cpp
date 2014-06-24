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
#include "Ctpars.h"
#include "VCameraRead.h"
#include "VDetectorTree.h"
#include "VGlobalRunParameter.h"

using namespace std;


// one tree per telescope type
map< ULong64_t, TTree* > fMapOfTrainingTree;

float isRMSZero( TTree* t, string iVar )
{
	if( !t || iVar.size() == 0 )
	{
		return -1.;
	}
	
	float f = 0.;
	float i_sum = 0.;
	float i_n = 0.;
	t->SetBranchAddress( iVar.c_str(), &f );
	
	// check first 1000 entries;
	int nentries = 1000;
	if( t->GetEntries() < nentries )
	{
		nentries = t->GetEntries();
	}
	
	for( int i = 0; i < nentries; i++ )
	{
		t->GetEntry( i );
		
		if( f > 0. )
		{
			i_sum += f;
			i_n++;
		}
	}
	
	if( i_n > 0. )
	{
		return i_sum / i_n;
	}
	
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
		if( !fMapOfTrainingTree_iter->second )
		{
			continue;
		}
		
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
		( TMVA::gConfig().GetIONames() ).fWeightFileDir = iOutputDirectory;
		
		// tmva regression
		TMVA::Factory* factory = new TMVA::Factory( "BDTDisp", &i_tmva, "V:!DrawProgressBar:!Color" );
		factory->AddVariable( "width", 'F' );
		factory->AddVariable( "length", 'F' );
		factory->AddVariable( "size", 'F' );
		factory->AddVariable( "tgrad_x*tgrad_x", 'F' );
		factory->AddVariable( "cross", 'F' );       
		factory->AddVariable( "asym", 'F' );
		factory->AddVariable( "loss", 'F' );		
		factory->AddTarget( "disp", 'F' );
//		factory->AddTarget( "dispEnergy", 'F' );
		
		factory->AddRegressionTree( fMapOfTrainingTree_iter->second, 1. );
		
		// quality cuts
		TCut fQualityCut = "size>100.&&ntubes>4.&&length>0.&&loss<0.2";
		
		factory->PrepareTrainingAndTestTree( fQualityCut, "nTrain_Regression=400000:nTest_Regression=40000:SplitMode=Random:NormMode=NumEvents:!V" );
		
		
		ostringstream iMVAName;
		iMVAName << "BDT_" << fMapOfTrainingTree_iter->first;
		cout << iMVAName.str() << endl;
		
		const char* htitle = iMVAName.str().c_str();
		
		factory->BookMethod( TMVA::Types::kBDT, htitle , "NTrees=2000::BoostType=Grad:Shrinkage=0.1:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=20:MaxDepth=6" );
		
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
bool writeTrainingFile( string iInputFile )
{

	// telescope configuration
	TChain i_telChain( "telconfig" );
	i_telChain.Add( iInputFile.c_str(), 0 );
	
	Ctelconfig i_tel( &i_telChain );
	i_tel.GetEntry( 0 );
	unsigned int i_ntel = i_tel.NTel;
	
	// training trees (one per telescope type)
	
            int runNumber = -1;
            int eventNumber = -1;
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
	    float Xcore = -1.;
	    float Ycore = -1.;
	    float Xoff = -1.;
	    float Yoff = -1.;
	    float LTrig = -1.;
	    float MCaz = -1.;
	    float disp = -1.;
	    float NImages = -1.;
	    float cross = -1.;
	    float dispPhi = -1.;
            float dispEnergy = -1.;
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
			
                          fMapOfTrainingTree[i_tel.TelType]->Branch( "runNumber", &runNumber, "runNumber/I" );
                          fMapOfTrainingTree[i_tel.TelType]->Branch( "eventNumber", &eventNumber, "eventNumber/I" );
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
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "Xcore", &Xcore, "Xcore/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "Ycore", &Ycore, "Ycore/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "Xoff", &Xoff, "Xoff/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "Yoff", &Yoff, "Yoff/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "LTrig", &LTrig, "LTrig/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "NImages", &NImages, "NImages/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "MCaz", &MCaz, "MCaz/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "Ze", &ze, "Ze/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "Az", &az, "Az/F " );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "disp", &disp, "disp/F" );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "cross", &cross, "cross/F" );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "dispPhi", &dispPhi, "dispPhi/F" );
			  fMapOfTrainingTree[i_tel.TelType]->Branch( "dispEnergy", &dispEnergy, "dispEnergy/F" );
			
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
		iTreeName << "Tel_" << i + 1 << "/tpars";
		TChain i_tparsTree( iTreeName.str().c_str() );
		i_tparsTree.Add( iInputFile.c_str(), 0 );
		Ctpars i_tpars( &i_tparsTree, true, 6, true );
		
		cout << "\t found tree " << iTreeName.str() << ", entries: " << i_showerpars.fChain->GetEntries() << ", ";
		cout << i_tpars.fChain->GetEntries() << endl;
		int nentries = i_showerpars.fChain->GetEntries();
		
		for( int n = 0; n < nentries; n++ )
		{
			i_showerpars.GetEntry( n );
			i_tpars.GetEntry( n );
			
			// quality cuts
			if( i_tpars.ntubes < 4 || i_tpars.size < 0 || i_tpars.loss > 0.2 )
			{
				continue;
			}
			
                          runNumber = i_showerpars.runNumber;
                          eventNumber = i_showerpars.eventNumber;
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
			  Xoff = i_showerpars.Xoff[0];
			  Yoff = i_showerpars.Yoff[0];
			  Xcore = i_showerpars.Xcore[0];
			  Ycore = i_showerpars.Ycore[0];
			  LTrig = i_showerpars.LTrig;
			  NImages = i_showerpars.NImages[0];
			  MCaz = i_showerpars.MCaz;
			
			// calculate disp (observe sign convention for MCyoff)
			disp = sqrt( ( cen_y + MCyoff ) * ( cen_y + MCyoff ) + ( cen_x - MCxoff ) * ( cen_x - MCxoff ) );
			cross = sqrt( (cen_y+Yoff)*(cen_y+Yoff) + (cen_x-Xoff)*(cen_x-Xoff) );
			dispPhi = TMath::ATan2( sinphi, cosphi ) - TMath::ATan2( cen_y + MCyoff, cen_x - MCxoff );

                        dispEnergy = i_showerpars.MCe0;
			
			if( fMapOfTrainingTree.find( i_tel.TelType ) != fMapOfTrainingTree.end() )
			{
				fMapOfTrainingTree[i_tel.TelType]->Fill();
			}
		}
	}
	
	
	return true;
}


int main( int argc, char* argv[] )
{
	// print version only
	if( argc == 2 )
	{
		string fCommandLine = argv[1];
		if( fCommandLine == "-v" || fCommandLine == "--version" )
		{
			VGlobalRunParameter fRunPara;
			cout << fRunPara.getEVNDISP_VERSION() << endl;
			exit( 0 );
		}
	}
	cout << endl;
        // print help text
	if( argc < 4 )
	{
		cout << "./trainTMVAforAngularReconstruction <input eventdisplay file (MC)> <output directory> <training file name>" << endl;
		cout << endl;
		exit( 0 );
	}
	string fInputFile = argv[1];
	string fOutputDir = argv[2];
	string fOutputName = argv[3];

        cout << "trainTMVAforAngularReconstruction (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
        cout << "------------------------------------" << endl;
        cout << endl;
        cout << "input evndisp file with training events: " << endl;
        cout << fInputFile << endl;
        cout << endl;
	
	// output file
	ostringstream iFileName;
	iFileName << fOutputDir << "/" << fOutputName << ".root";
	TFile iO( iFileName.str().c_str(), "RECREATE" );
	if( iO.IsZombie() )
	{
		cout << "Error creating output file: " << iFileName.str() << endl;
                cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	// fill training file
	if( !writeTrainingFile( fInputFile ) )
	{
		cout << "error writing training file " << endl;
                cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	// write training tree to output file
	map< ULong64_t, TTree* >::iterator fMapOfTrainingTree_iter;
	for( fMapOfTrainingTree_iter = fMapOfTrainingTree.begin(); fMapOfTrainingTree_iter != fMapOfTrainingTree.end(); fMapOfTrainingTree_iter++ )
	{
		if( fMapOfTrainingTree_iter->second )
		{
			fMapOfTrainingTree_iter->second->Write();
		}
	}
	
	// train TMVA
	trainTMVA( fOutputDir, fOutputName );
	
	// close output file
	iO.Close();
	
}
