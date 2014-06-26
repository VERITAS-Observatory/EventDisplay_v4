/*! \file combineEffectiveAreas.cpp
    \brief combine effective areas calculated for a one combination of ze,az,woff... into a single large file


    \author Gernot Maier
*/

#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>
#include <stdlib.h>
#include <string>

#include <VGammaHadronCuts.h>
#include <VGlobalRunParameter.h>
#include <VInstrumentResponseFunctionRunParameter.h>

using namespace std;


void merge( string ifile, char* outputfile, bool bFull = false )
{
	char hname[2000];
	
	TChain f( "fEffArea" );
	if( ifile.find( ".root" ) != string::npos )
	{
		sprintf( hname, "%s", ifile.c_str() );
	}
	else
	{
		sprintf( hname, "%s.root", ifile.c_str() );
	}
	int i_nMerged = f.Add( hname );
	if( i_nMerged == 0 )
	{
		cout << "error: no files found to merge: " << endl;
		cout << "\t" << hname << endl;
		cout << "exiting.." << endl;
		exit( EXIT_FAILURE );
	}
	sprintf( hname, "%s.root", outputfile );
	cout << "merging " << i_nMerged << " files to " << hname << endl;
	
	// set branches to be included in merged files
	if( !bFull )
	{
		f.SetBranchStatus( "*", 0 );
		f.SetBranchStatus( "ze", 1 );
		f.SetBranchStatus( "az", 1 );
		f.SetBranchStatus( "azMin", 1 );
		f.SetBranchStatus( "azMax", 1 );
		f.SetBranchStatus( "Xoff", 1 );
		f.SetBranchStatus( "Yoff", 1 );
		f.SetBranchStatus( "Woff", 1 );
		f.SetBranchStatus( "noise", 1 );
		f.SetBranchStatus( "pedvar", 1 );
		f.SetBranchStatus( "index", 1 );
		f.SetBranchStatus( "nbins", 1 );
		f.SetBranchStatus( "e0", 1 );
		f.SetBranchStatus( "eff", 1 );
		// errors not needed for standard analysis
		f.SetBranchStatus( "seff_L", 1 );
		f.SetBranchStatus( "seff_U", 1 );
		f.SetBranchStatus( "Rec_nbins", 1 );
		f.SetBranchStatus( "Rec_e0", 1 );
		f.SetBranchStatus( "Rec_eff", 1 );
		// errors not needed for standard analysis
		//        f.SetBranchStatus( "Rec_seff_L", 1 );
		//        f.SetBranchStatus( "Rec_seff_U", 1 );
		// needed for compatibility to v3.30
		//        f.SetBranchStatus( "hEmc", 1 );
		// needed for systematic error calculation
		f.SetBranchStatus( "hEsysMCRelative", 1 );
	}
	f.Merge( hname );
	cout << "done.." << endl;
	
	// get one example of hEmc
	// (this is needed later to get the binning right)
	TFile* fO = new TFile( hname, "UPDATE" );
	if( fO->IsZombie() )
	{
		cout << "error writing hEmc to output file" << endl;
		return;
	}
	TH1D* hEmc = 0;
	f.SetBranchAddress( "hEmc", &hEmc );
	f.GetEntry( 0 );
	fO->cd();
	if( hEmc )
	{
		hEmc->Reset();
		hEmc->Write();
	}
	// get one example of IRF-runparameters for later checks in the analysis
	// (this assumes they are the same in all merged files!)
	TFile* ifirst = f.GetFile();
	if( !ifirst )
	{
		cout << "error finding pointer to first file in chain" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	VInstrumentResponseFunctionRunParameter* iRunPara = ( VInstrumentResponseFunctionRunParameter* )ifirst->Get( "makeEffectiveArea_runparameter" );
	if( !iRunPara )
	{
		cout << "error copying VInstrumentResponseFunctionRunParameter to output file" << endl;
		cout << "could not find them in file: " << ifirst->GetName() << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	iRunPara->Write();
	// get one example of the gamma-hadron cuts
	// (this assume they are the same in all merged files!)
	VGammaHadronCuts* iCuts = ( VGammaHadronCuts* )ifirst->Get( "GammaHadronCuts" );
	if( iCuts )
	{
		cout << "copying gamma/hadron cuts from first file (" << ifirst->GetName() << ") into the output file" << endl;
		iCuts->Write();
	}
	else
	{
		cout << "error copying gamma/hadron cuts into output file" << endl;
		cout << "could not find them in file: " << ifirst->GetName() << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	fO->Close();
	
	// merge all log files
	if( ifile.find( ".root" ) != string::npos )
	{
		sprintf( hname, "cat %s*.log > %s.combine.log", ifile.substr( 0, ifile.size()-5 ).c_str(), outputfile );
	}
	else
	{
		sprintf( hname, "cat %s*.log > %s.combine.log", ifile.c_str(), outputfile );
	}
	cout << "merge log files into " << hname << endl;
	system( hname );
	cout << "done..";
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
	if( argc < 4 )
	{
		cout << endl;
		cout << "combineEffectiveAreas <effective area files> <combined file> <write all histograms (default value is=false)>" << endl;
		cout << endl;
		cout << "   <effective area files>    without .root suffix (e.g. effArea*. Note need of \"...\")" << endl;
		cout << endl;
		exit( 0 );
	}
	cout << endl;
	cout << "combineEffectiveAreas (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
	cout << "------------------------------------" << endl;
	cout << endl;
	
	merge( argv[1], argv[2], ( bool )atoi( argv[3] ) );
	
	cout << endl << "end combineEffectiveAreas" << endl;
	
}

