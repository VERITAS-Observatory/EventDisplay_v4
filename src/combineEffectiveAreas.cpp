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

void write_reduced_merged_tree( string file_name, 
                                TChain *f )
{
    if( f )
    {
       cout << "AAA " << f->GetName() << endl;
    }


}

void merge( string ifile, string outputfile, string tree_type = "DL3" )
{
	TChain f( "fEffArea" );
	if( ifile.find( ".root" ) == string::npos )
	{
                ifile += ".root";
        }
	int i_nMerged = f.Add( ifile.c_str() );
	if( i_nMerged == 0 )
	{
		cout << "error: no files found to merge: " << endl;
		cout << "\t" << ifile << endl;
		cout << "exiting.." << endl;
		exit( EXIT_FAILURE );
	}
        if( outputfile.find( ".root" ) == string::npos )
        {
            outputfile += ".root";
        }
	cout << "merging " << i_nMerged << " files to " << outputfile << endl;
	
	// set branches to be included in merged files
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
        f.SetBranchStatus( "esys_rel", 1 );
        f.SetBranchStatus( "Rec_nbins", 1 );
        f.SetBranchStatus( "Rec_e0", 1 );
        f.SetBranchStatus( "Rec_eff", 1 );
        f.SetBranchStatus( "hEsysMCRelative", 1 ); 
        if( tree_type == "DL3" )
        {
            f.SetBranchStatus( "effNoTh2", 1 );
            f.SetBranchStatus( "Rec_effNoTh2", 1 );
            f.SetBranchStatus( "Rec_angRes_p68", 1 );
            f.SetBranchStatus( "Rec_angRes_p80", 1 );
            // Full histograms for DL3
            f.SetBranchStatus( "hEsysMCRelative2D", 1 );
            f.SetBranchStatus( "hEsysMCRelative2DNoDirectionCut", 1 );
            f.SetBranchStatus( "hAngularLogDiffEmc_2D", 1 );
        }
	f.Merge( outputfile.c_str() );
	cout << "done.." << endl;
        if( tree_type == "DL3reduced" )
        {
            write_reduced_merged_tree( outputfile, &f );
         }
	
	// get one example of hEmc
	// (this is needed later to get the binning right)
	TFile* fO = new TFile( outputfile.c_str(), "UPDATE" );
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
        ostringstream i_sys;
	if( ifile.find( ".root" ) != string::npos )
	{
                i_sys << "cat " << ifile.substr( 0, ifile.size() - 5 ).c_str() << "*.log > ";
	}
	else
	{
                i_sys << "cat " << ifile << "*.log > ";
	}
        i_sys << outputfile << ".combine.log";
	cout << "merge log files into " << i_sys.str() << endl;
	system( i_sys.str().c_str() );
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
			exit( EXIT_FAILURE );
		}
	}
	if( argc < 4 )
	{
		cout << endl;
		cout << "combineEffectiveAreas <effective area files> <combined file> <tree type>" << endl; 
                cout << endl;
                cout << "  <tree type>  effective area tree type (defines size of combined tree)" << endl;
                cout << "                - DL3 (default): entries required for DL3 analyis (large)" << endl;
                cout << "                - all          : all entries of original trees (largest)" << endl;
                cout << "                - anasum       : entries required for anasum analysis only (smallest)" << endl;
		cout << endl;
		cout << "   <effective area files>    without .root suffix (e.g. effArea*. Note need of \"...\")" << endl;
		cout << endl;
		exit( EXIT_FAILURE );
	}
	cout << endl;
	cout << "combineEffectiveAreas (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
	cout << "------------------------------------" << endl;
	cout << endl;
	
	merge( argv[1], argv[2], argv[3] );
	
	cout << endl << "end combineEffectiveAreas" << endl;
	
}

