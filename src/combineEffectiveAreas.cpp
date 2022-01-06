/*! \file combineEffectiveAreas.cpp
    \brief combine effective areas calculated for a one combination of ze,az,woff... into a single large file

    Testing:
    ./bin/combineEffectiveAreas "/lustre/fs23/group/veritas/IRFPRODUCTION/v486/CARE_June2020/V6_2012_2013a_ATM61_gamma/EffectiveAreas_DL3/Eff*" tt.root DL3reduced


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

/*
 * convert TH2F histograms into arrays
 *
 *  hEsysMCRelative2D, hEsysMCRelative2DNoDirectionCut
 *  x-axis: energy 30 bins
 *  y-axis: energy bias E_{rec}/E_{MC} 75 
 *
 *  r e.g., hEsysMCRelative2D:
 *
 *  for the axes:
 *
 *  nbinsx --> number of bins
 *  minx --> minimum of x-axis
 *  maxx --> maximum of x-axis
 *  (same for y-axis)
 *  nbinsxy -> nbinsx x nbinsy
 *  value -> histogram content (array of nbinsxy )
 *
 *  Meaning that the 2D histo is filled into a 1D array:
 *
 *  2d_array[0..nbinsx][0] = 1D_array[0… nbinsx]
 *  ....
 *  2d_array[0..nbinsx][3] = 1D_array[3 x nbinsx + 0… nbinsx]
 *  and so on
 */
bool write_reduced_merged_tree( string inputfile,
                                string outputfile,
                                string tree_type = "DL3" )
{
    if( tree_type != "DL3reduced" ) return true;

    TChain f( "fEffArea" );
    if( inputfile.find( ".root" ) == string::npos )
    {
            inputfile += ".root";
    }
    int i_nMerged = f.Add( inputfile.c_str() );
    if( !i_nMerged )
    {
       cout << "error writing reduced merged tree contents" << endl;
       return false;
    }
    vector< string > hist_names;
    hist_names.push_back( "hEsysMCRelative2D" );
    hist_names.push_back( "hEsysMCRelative2DNoDirectionCut" );
    hist_names.push_back( "hAngularLogDiffEmc_2D" );
    vector< TH2F* > hist_to_read( hist_names.size(), 0 );
    vector< int > nbinsx( hist_names.size(), 0 );
    vector< float > min_x( hist_names.size(), 0. );
    vector< float > max_x( hist_names.size(), 0. );
    vector< int > nbinsy( hist_names.size(), 0 );
    vector< float > min_y( hist_names.size(), 0. );
    vector< float > max_y( hist_names.size(), 0. );
    vector< int > nbinsxy( hist_names.size(), 0 );
    const int max_nxy = 10000;
    vector< float* > hist_value( hist_names.size(), new float[max_nxy] );

    // initialize
    for( unsigned int i = 0; i < hist_names.size(); i++ )
    {
        f.SetBranchAddress( hist_names[i].c_str(),
                           &hist_to_read[i] );
    }

    if( outputfile.find( ".root" ) == string::npos ) outputfile += ".root";
    TFile* fO = new TFile( outputfile.c_str(), "UPDATE" );
    if( fO->IsZombie() )
    {
            cout << "error writing hEmc to output file" << endl;
            return false;
    }
    TTree *t = new TTree( "fEffAreaH2F",
                          "effective area tree (2D histograms)" );
  
    // initialize branches
    // assume that histograms are equivalent for all entries
    f.GetEntry( 0 );
    for( unsigned int h = 0; h < hist_names.size(); h++ )
    {
        t->Branch( (hist_names[h]+"_binsx").c_str(),
                   &nbinsx[h],
                   (hist_names[h]+"_binsx/I").c_str() );
        t->Branch( (hist_names[h]+"_minx").c_str(),
                   &min_x[h],
                   (hist_names[h]+"_minx/F").c_str() );
        t->Branch( (hist_names[h]+"_maxx").c_str(),
                   &max_x[h],
                   (hist_names[h]+"_maxx/F").c_str() );
        if( hist_to_read[h] )
        {
            nbinsx[h] = hist_to_read[h]->GetXaxis()->GetNbins();
            min_x[h] = hist_to_read[h]->GetXaxis()->GetXmin();
            max_x[h] = hist_to_read[h]->GetXaxis()->GetXmax();
        }
        t->Branch( (hist_names[h]+"_binsy").c_str(),
                   &nbinsy[h],
                   (hist_names[h]+"_binsy/I").c_str() );
        t->Branch( (hist_names[h]+"_miny").c_str(),
                   &min_y[h],
                   (hist_names[h]+"_miny/F").c_str() );
        t->Branch( (hist_names[h]+"_maxy").c_str(),
                   &max_y[h],
                   (hist_names[h]+"_maxy/F").c_str() );
        if( hist_to_read[h] )
        {
            nbinsy[h] = hist_to_read[h]->GetYaxis()->GetNbins();
            min_y[h] = hist_to_read[h]->GetYaxis()->GetXmin();
            max_y[h] = hist_to_read[h]->GetYaxis()->GetXmax();
        }
        t->Branch( (hist_names[h]+"_binsxy").c_str(),
                   &nbinsxy[h],
                   (hist_names[h]+"_binsxy/I").c_str() );
        nbinsxy[h] = nbinsx[h] * nbinsy[h];
        if( nbinsxy[h] > max_nxy )
        {
            cout << "error: histogram dimensions larger than hardwired values" << endl;
            exit( EXIT_FAILURE );
        }
        t->Branch( (hist_names[h]+"_value").c_str(),
                   hist_value[h],
                   (hist_names[h]+"_value["+hist_names[h]+"_binsxy]/F").c_str() );
    }

    // loop over all entries and copy histograms to arrays
    Long64_t nentries = f.GetEntries();
    cout << "prepare reduced arrays for " << nentries << " entries" << endl;
    int nxy = 0;
    for( unsigned int i = 0; i < nentries; i++ )
    {
        f.GetEntry( i );
        for( unsigned int h = 0; h < hist_to_read.size(); h++ )
        {
            if( !hist_to_read[h] ) continue;
            for( int nx = 0; nx < hist_to_read[h]->GetXaxis()->GetNbins(); nx++ )
            {
                for( int ny = 0; ny < hist_to_read[h]->GetYaxis()->GetNbins(); ny++ )
                {
                    nxy = ny*hist_to_read[h]->GetXaxis()->GetNbins() + nx;
                    hist_value[h][nxy] = hist_to_read[h]->GetBinContent( nx, ny );
                }
            }
        }
        t->Fill();
     }
     t->Write();
     fO->Close();
     return true;
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
	VInstrumentResponseFunctionRunParameter* iRunPara = 
                   ( VInstrumentResponseFunctionRunParameter* )ifirst->Get( "makeEffectiveArea_runparameter" );
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
		cout << "copying gamma/hadron cuts from first file (";
                cout << ifirst->GetName() << ") into the output file" << endl;
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
}

void write_log_files( string ifile, string outputfile )
{
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
	cout << "done.." << endl;
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
        write_reduced_merged_tree( argv[1], argv[2], argv[3] );
        write_log_files( argv[1], argv[2] );
	
	cout << endl << "end combineEffectiveAreas" << endl;
}
