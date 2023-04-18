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
 *  energy axis is in all case 'true energy'
 *  we skip therefore
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
bool write_reduced_merged_tree( vector< string > file_list,
								string outputfile,
								string tree_type = "DL3" )
{
	if( tree_type != "DL3reduced" && tree_type != "DL3test" )
	{
		return true;
	}
	
	if( file_list.size() == 0 )
	{
		cout << "error: no files found to merge" << endl;
		cout << "exiting.." << endl;
		exit( EXIT_FAILURE );
	}
	TChain f( "fEffArea" );
	for( unsigned int i = 0; i < file_list.size(); i++ )
	{
		f.Add( file_list[i].c_str() );
	}
	
	// observational parameters and effective areas
	Double_t ze = 0.;
	Int_t az = 0;
	Double_t azMin = 0.;
	Double_t azMax = 0.;
	Double_t Woff = 0.;
	Double_t index = 0.;
	Int_t noise = 0;
	Double_t pedvar = 0.;
	Int_t nbins = 0;
	Double_t e0[1000];
	Double_t eff[1000];
	Double_t effNoTh2[1000];
	Float_t esys_rel[1000];
	TH1D* hEcut = 0;
	TProfile* hEsysMCRelative = 0;
	f.SetBranchAddress( "ze", &ze );
	f.SetBranchAddress( "az", &az );
	f.SetBranchAddress( "azMin", &azMin );
	f.SetBranchAddress( "azMax", &azMax );
	f.SetBranchAddress( "Woff", &Woff );
	f.SetBranchAddress( "noise", &noise );
	f.SetBranchAddress( "pedvar", &pedvar );
	f.SetBranchAddress( "nbins", &nbins );
	f.SetBranchAddress( "index", &index );
	f.SetBranchAddress( "e0", e0 );
	f.SetBranchAddress( "eff", eff );
	f.SetBranchAddress( "effNoTh2", effNoTh2 );
	f.SetBranchAddress( "esys_rel", esys_rel );
	f.SetBranchAddress( "hEcut", &hEcut );
	f.SetBranchAddress( "hEsysMCRelative", &hEsysMCRelative );
	double min_index = 1.e10;
	for( unsigned int i = 0; i < 1000; i++ )
	{
		f.GetEntry( i );
		if( index < min_index )
		{
			min_index = index;
		}
	}
	cout << "Min index for IRFs in true energy: " << min_index << endl;
	if( hEcut )
	{
		cout << "Basic binning: " << hEcut->GetNbinsX();
		cout << " [" << hEcut->GetXaxis()->GetXmin();
		cout << ", " << hEcut->GetXaxis()->GetXmax() << "]" << endl;
	}
	else
	{
		cout << "Error: histogram hEcut not found for binning determination" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	
	// histograms
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
	vector< float* > hist_value;
	for( unsigned int i = 0; i < hist_names.size(); i++ )
	{
		hist_value.push_back( new float[max_nxy] );
	}
	
	// initialize reading tree
	for( unsigned int i = 0; i < hist_names.size(); i++ )
	{
		f.SetBranchAddress( hist_names[i].c_str(),
							&hist_to_read[i] );
	}
	// output file
	if( outputfile.find( ".root" ) == string::npos )
	{
		outputfile += ".root";
	}
	TFile* fO = new TFile( outputfile.c_str(), "UPDATE" );
	if( fO->IsZombie() )
	{
		cout << "error writing hEmc to output file" << endl;
		return false;
	}
	// initialize writing tree
	TTree* t = new TTree( "fEffAreaH2F",
						  "effective area tree (2D histograms)" );
	Float_t t_ze = 0;
	UShort_t t_az = 0;
	Float_t t_azMin = 0.;
	Float_t t_azMax = 0.;
	Float_t t_Woff = 0.;
	UShort_t t_noise = 0;
	Float_t t_pedvar = 0.;
	UShort_t t_nbins = hEcut->GetNbinsX();
	Float_t t_e0[1000];
	Float_t t_eff[1000];
	Float_t t_effNoTh2[1000];
	UShort_t t_esys_nbins = hEsysMCRelative->GetNbinsX();
	Float_t t_esys_e0[1000];
	Float_t t_esys_rel[1000];
	for( unsigned int i = 0; i < t_nbins; i++ )
	{
		t_e0[i] = hEcut->GetXaxis()->GetBinCenter( i + 1 );
		t_eff[i] = 0.;
		t_effNoTh2[i] = 0.;
	}
	for( unsigned int i = 0; i < t_esys_nbins; i++ )
	{
		t_esys_e0[i] = hEsysMCRelative->GetXaxis()->GetBinCenter( i + 1 );
		t_esys_rel[i] = 0.;
	}
	t->Branch( "ze", &t_ze, "ze/F" );
	t->Branch( "az", &t_az, "az/s" );
	t->Branch( "azMin", &t_azMin, "azMin/F" );
	t->Branch( "azMax", &t_azMax, "azMax/F" );
	t->Branch( "Woff", &t_Woff, "Woff/F" );
	t->Branch( "noise", &t_noise, "noise/s" );
	t->Branch( "pedvar", &t_pedvar, "pedvar/F" );
	t->Branch( "nbins", &t_nbins, "nbins/s" );
	t->Branch( "e0", t_e0, "e0[nbins]/F" );
	t->Branch( "eff", t_eff, "eff[nbins]/F" );
	t->Branch( "effNoTh2", t_effNoTh2, "effNoTh2[nbins]/F" );
	t->Branch( "nbins_esys", &t_esys_nbins, "nbins_esys/s" );
	t->Branch( "e0_esys", t_esys_e0, "e0_esys[nbins_esys]/F" );
	t->Branch( "esys_rel", t_esys_rel, "esys_rel[nbins_esys]/F" );
	
	// initialize branches
	// assume that histograms are equivalent for all entries
	f.GetEntry( 0 );
	for( unsigned int h = 0; h < hist_names.size(); h++ )
	{
		t->Branch( ( hist_names[h] + "_binsx" ).c_str(),
				   &nbinsx[h],
				   ( hist_names[h] + "_binsx/I" ).c_str() );
		t->Branch( ( hist_names[h] + "_minx" ).c_str(),
				   &min_x[h],
				   ( hist_names[h] + "_minx/F" ).c_str() );
		t->Branch( ( hist_names[h] + "_maxx" ).c_str(),
				   &max_x[h],
				   ( hist_names[h] + "_maxx/F" ).c_str() );
		if( hist_to_read[h] )
		{
			nbinsx[h] = hist_to_read[h]->GetXaxis()->GetNbins();
			min_x[h] = hist_to_read[h]->GetXaxis()->GetXmin();
			max_x[h] = hist_to_read[h]->GetXaxis()->GetXmax();
		}
		t->Branch( ( hist_names[h] + "_binsy" ).c_str(),
				   &nbinsy[h],
				   ( hist_names[h] + "_binsy/I" ).c_str() );
		t->Branch( ( hist_names[h] + "_miny" ).c_str(),
				   &min_y[h],
				   ( hist_names[h] + "_miny/F" ).c_str() );
		t->Branch( ( hist_names[h] + "_maxy" ).c_str(),
				   &max_y[h],
				   ( hist_names[h] + "_maxy/F" ).c_str() );
		if( hist_to_read[h] )
		{
			nbinsy[h] = hist_to_read[h]->GetYaxis()->GetNbins();
			min_y[h] = hist_to_read[h]->GetYaxis()->GetXmin();
			max_y[h] = hist_to_read[h]->GetYaxis()->GetXmax();
		}
		t->Branch( ( hist_names[h] + "_binsxy" ).c_str(),
				   &nbinsxy[h],
				   ( hist_names[h] + "_binsxy/I" ).c_str() );
		nbinsxy[h] = nbinsx[h] * nbinsy[h];
		if( nbinsxy[h] > max_nxy )
		{
			cout << "error: histogram dimensions larger than hardwired values" << endl;
			exit( EXIT_FAILURE );
		}
		t->Branch( ( hist_names[h] + "_value" ).c_str(),
				   hist_value[h],
				   ( hist_names[h] + "_value[" + hist_names[h] + "_binsxy]/F" ).c_str() );
	}
	
	// loop over all entries and copy histograms to arrays
	Long64_t nentries = f.GetEntries();
	cout << "prepare reduced arrays for " << nentries << " entries" << endl;
	int nxy = 0;
	int ntemp_bin = 0;
	for( unsigned int i = 0; i < nentries; i++ )
	{
		f.GetEntry( i );
		// IRFs as function of energy without spectral index dependence
		if( TMath::Abs( index - min_index ) > 1.e-3 )
		{
			continue;
		}
		t_ze = ze;
		t_az = az;
		t_azMin = azMin;
		t_azMax = azMax;
		t_Woff = Woff;
		t_noise = noise;
		t_pedvar = pedvar;
		// padding with zeros
		for( int b = 0; b < t_nbins; b++ )
		{
			t_eff[b] = 0.;
			t_effNoTh2[b] = 0.;
		}
		for( int b = 0; b < t_esys_nbins; b++ )
		{
			t_esys_rel[b] = 0.;
		}
		for( int b = 0; b < nbins; b++ )
		{
			ntemp_bin = hEcut->GetXaxis()->FindBin( e0[b] );
			if( ntemp_bin > 0 && ntemp_bin < t_nbins )
			{
				t_eff[ntemp_bin - 1] = eff[b];
				t_effNoTh2[ntemp_bin - 1] =  effNoTh2[b];
			}
		}
		// bias in energy reconstruction
		for( int b = 0; b < t_esys_nbins; b++ )
		{
			t_esys_rel[b] = hEsysMCRelative->GetBinContent( b + 1 );
		}
		
		// histograms
		for( unsigned int h = 0; h < hist_to_read.size(); h++ )
		{
			if( !hist_to_read[h] )
			{
				continue;
			}
			for( int nx = 0; nx < hist_to_read[h]->GetXaxis()->GetNbins(); nx++ )
			{
				for( int ny = 0; ny < hist_to_read[h]->GetYaxis()->GetNbins(); ny++ )
				{
					nxy = ny * hist_to_read[h]->GetXaxis()->GetNbins() + nx;
					hist_value[h][nxy] = hist_to_read[h]->GetBinContent( nx + 1, ny + 1 );
				}
			}
		}
		t->Fill();
	}
	cout << "\t reduced array tree size: " << t->GetEntries() << endl;
	t->Write();
	fO->Close();
	return true;
}

void merge( vector< string > file_list,
			string outputfile,
			string tree_type = "DL3" )
{
	if( file_list.size() == 0 )
	{
		cout << "error: no files found to merge" << endl;
		cout << "exiting.." << endl;
		exit( EXIT_FAILURE );
	}
	TChain f( "fEffArea" );
	for( unsigned int i = 0; i < file_list.size(); i++ )
	{
		f.Add( file_list[i].c_str() );
	}
	if( outputfile.find( ".root" ) == string::npos )
	{
		outputfile += ".root";
	}
	cout << "merging " << file_list.size() << " files to " << outputfile << endl;
	
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
	if( tree_type != "DL3reduced" )
	{
		f.SetBranchStatus( "nbins", 1 );
		f.SetBranchStatus( "e0", 1 );
		f.SetBranchStatus( "esys_rel", 1 );
		f.SetBranchStatus( "eff", 1 );
		f.SetBranchStatus( "effNoTh2", 1 );
	}
	f.SetBranchStatus( "Rec_nbins", 1 );
	f.SetBranchStatus( "Rec_e0", 1 );
	f.SetBranchStatus( "Rec_eff", 1 );
	// f.SetBranchStatus( "hEsysMCRelative", 1 );
	if( tree_type == "DL3" || tree_type == "DL3test" )
	{
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

void write_log_files( vector< string > file_list, string outputfile )
{
	// merge all log files
	ostringstream i_sys;
	for( unsigned int i = 0; i < file_list.size(); i++ )
	{
		if( file_list[i].find( ".root" ) != string::npos )
		{
			i_sys << "cat " << file_list[i].substr( 0, file_list[i].size() - 5 ).c_str() << ".log > ";
		}
		else
		{
			i_sys << "cat " << file_list[i] << ".log > ";
		}
	}
	
	i_sys << outputfile << ".combine.log";
	cout << "merge log files into " << i_sys.str() << endl;
	int sys_ret = system( i_sys.str().c_str() );
	if( sys_ret == -1 )
	{
		cout << "Error executing system command " << i_sys.str() << endl;
	}
	cout << "done.." << endl;
}

/*
 * return list of effective area files
 * to be merged
 *
 */
vector< string > readListOfFiles( string iFile )
{
	vector< string > iList;
	
	ifstream is;
	is.open( iFile.c_str() );
	if( !is )
	{
		cout << "error while reading file list " << iFile << endl;
		cout << "exiting...." << endl;
		exit( EXIT_FAILURE );
	}
	string is_line;
	
	while( getline( is, is_line ) )
	{
		iList.push_back( is_line );
	}
	
	is.close();
	
	return iList;
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
		cout << "combineEffectiveAreas <effective area file list> <combined file> <tree type>" << endl;
		cout << endl;
		cout << "  <effective area file list>  list of effective files to be merged" << endl;
		cout << "  <tree type>  effective area tree type (defines size of combined tree)" << endl;
		cout << "                - DL3 (default): entries required for DL3 analyis (large)" << endl;
		cout << "                - all          : all entries of original trees (largest)" << endl;
		cout << "                - anasum       : entries required for anasum analysis only (smallest)" << endl;
		cout << "                - DL3reduced   : histograms are written as regular arrays for DL3 analysis" << endl;
		cout << endl;
		cout << endl;
		exit( EXIT_FAILURE );
	}
	cout << endl;
	cout << "combineEffectiveAreas (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
	cout << "------------------------------------" << endl;
	cout << endl;
	
	vector< string > file_list = readListOfFiles( argv[1] );
	
	merge( file_list, argv[2], argv[3] );
	write_reduced_merged_tree( file_list, argv[2], argv[3] );
	// write_log_files( file_list, argv[2] );
	
	cout << endl << "end combineEffectiveAreas" << endl;
}
