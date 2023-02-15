/*! \file VTS.calculateCrabRateFromMC.cpp
    \brief calculate gamma ray rate from Crab Nebula with effective areas from MC



*/

#include "TF1.h"
#include "TFile.h"
#include "TGraph2DErrors.h"
#include "TProfile2D.h"
#include "TTree.h"

#include "CEffArea.h"
#include "VGlobalRunParameter.h"
#include "VMonteCarloRateCalculator.h"

#include <iostream>
#include <vector>


using namespace std;

/*
 * fill profile histogram with a new rate entry
 *
 */
void fill_profilehistogram_for_TMVA(
	TProfile2D* fMCRates_TMVA,
	double i_E, double i_Ze, double iRate )
{
	if( fMCRates_TMVA && iRate > 0. )
	{
		fMCRates_TMVA->Fill( i_E, i_Ze, iRate );
	}
}

/*
 * read zenith bin vector from MC tree
 *
 */
vector< double > read_zenith_bins( string fEffAreaFile )
{
	TFile* f = new TFile( fEffAreaFile.c_str() );
	if( f->IsZombie() )
	{
		cout << "error opening file with effective areas " << fEffAreaFile << endl;
		exit( EXIT_FAILURE );
	}
	TTree* t = ( TTree* )gDirectory->Get( "fEffAreaH2F" );
	if( !t )
	{
		cout << "error reading effective area tree from " << fEffAreaFile << endl;
		exit( EXIT_FAILURE );
	}
	float ze = 0.;
	t->SetBranchAddress( "ze", &ze );
	// - read unique vector of zenith angles
	vector< double > tmp_zebins;
	for( unsigned int i = 0; i < t->GetEntries(); i++ )
	{
		t->GetEntry( i );
		bool bFound = false;
		for( unsigned int z = 0; z < tmp_zebins.size(); z++ )
		{
			if( TMath::Abs( ze - tmp_zebins[z] ) < 1.e-1 )
			{
				bFound = true;
				break;
			}
		}
		if( !bFound )
		{
			tmp_zebins.push_back( ze );
		}
	}
	tmp_zebins.push_back( tmp_zebins.back() + 10. );
	vector< double > tmp_zebin_edges;
	tmp_zebin_edges.push_back( 0. );
	for( unsigned int z = 1; z < tmp_zebins.size(); z++ )
	{
		tmp_zebin_edges.push_back( 0.5 * ( tmp_zebins[z] + tmp_zebins[z - 1] ) );
	}
	cout << "Zenith bins: ";
	for( unsigned int z = 0; z < tmp_zebin_edges.size(); z++ )
	{
		cout << tmp_zebin_edges[z] << ", ";
	}
	cout << endl;
	
	f->Close();
	
	return tmp_zebin_edges;
}

/*
 * read energy bin vector from TMVA run parameter file
 *
 */
vector< double > read_energy_bins( string iTMVAParameterFile )
{
	vector< double > tmp_e;
	ifstream is( iTMVAParameterFile.c_str(), ifstream::in );
	if( !is )
	{
		cout << "Error reading energy bins from TMVA run parameter file" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	string is_line;
	string temp;
	while( getline( is, is_line ) )
	{
		if( is_line.size() == 0 )
		{
			continue;
		}
		istringstream is_stream( is_line );
		if( ( is_stream >> std::ws ).eof() )
		{
			continue;
		}
		is_stream >> temp;
		if( temp != "*" )
		{
			continue;
		}
		is_stream >> temp;
		if( temp == "ENERGYBINS" )
		{
			is_stream >> temp;
			while( !( is_stream >> std::ws ).eof() )
			{
				double iT = 0.;
				is_stream >> iT;
				tmp_e.push_back( iT );
			}
		}
	}
	is.close();
	
	cout << "Energy bins: ";
	for( unsigned int e = 0; e < tmp_e.size(); e++ )
	{
		cout << tmp_e[e] << ", ";
	}
	cout << endl;
	
	return tmp_e;
}

/*
  initialize profile histograms rates as function of energy and zenith angle
   - entry 0: MC rates from the Crab nebula

*/
vector< TProfile2D* > initializeRateProfileHistos( string fEffAreaFile, string fTMVAParameterFile )
{
	vector< TProfile2D* > h;
	
    /* TODO
     * - allow to take overlapping energy bins into account
     * - two types of bins:
     *   - for histogram (non overlapping)
     *   - for integration (possibly overlapping)
     *
     */
	vector< double > tmp_ebins = read_energy_bins( fTMVAParameterFile );
	
	vector< double > tmp_zebin_edges = read_zenith_bins( fEffAreaFile );
	
	double* ie = &tmp_ebins[0];
	double* iz = &tmp_zebin_edges[0];
	h.push_back( new TProfile2D(
					 "MCRatesTMVA", "",
					 ( int )tmp_ebins.size() - 1, ie,
					 ( int )tmp_zebin_edges.size() - 1, iz ) );
	h.back()->SetXTitle( "log_{10} Energy/TeV" );
	h.back()->SetYTitle( "zenith angle (deg)" );
	h.back()->SetZTitle( "rate from Crab Nebula (1/min)" );
	
	h.push_back( new TProfile2D(
					 "BackgroundRatesTMVA", "",
					 ( int )tmp_ebins.size() - 1, ie,
					 ( int )tmp_zebin_edges.size() - 1, iz ) );
	h.back()->SetXTitle( "log_{10} Energy/TeV" );
	h.back()->SetYTitle( "zenith angle (deg)" );
	h.back()->SetZTitle( "background rate (1/min)" );
	
	return h;
}

/*
 * fill MC rates from effective area trees
 *
 */
TTree* fillMCRates( string fEffAreaFile, TProfile2D* h, double fEnergyThreshold )
{
	float ze = 0.;
	int az = 0;
	float Woff = 0.;
	int noise = 0;
	float pedvar = 0.;
	float MCrate;
	
	TTree* fMC = new TTree( "fMCRate", "MC rate predictions" );
	fMC->Branch( "ze", &ze, "ze/F" );
	fMC->Branch( "az", &az, "az/I" );
	fMC->Branch( "Woff", &Woff, "Woff/F" );
	fMC->Branch( "noise", &noise, "noise/I" );
	fMC->Branch( "pedvar", &pedvar, "pedvar/F" );
	fMC->Branch( "MCrate", &MCrate, "MCrate/F" );
	
	TFile* f = new TFile( fEffAreaFile.c_str() );
	if( f->IsZombie() )
	{
		cout << "error opening file with effective areas " << fEffAreaFile << endl;
		exit( EXIT_FAILURE );
	}
	TTree* t = ( TTree* )gDirectory->Get( "fEffAreaH2F" );
	if( !t )
	{
		cout << "error reading effective area tree from " << fEffAreaFile << endl;
		exit( EXIT_FAILURE );
	}
	UShort_t t_az = 0;
	UShort_t t_noise = 0;
	UShort_t t_nbins = 0;
	float t_e0[1000];
	float t_eff[1000];
	t->SetBranchAddress( "ze", &ze );
	t->SetBranchAddress( "az", &t_az );
	t->SetBranchAddress( "Woff", &Woff );
	t->SetBranchAddress( "noise", &t_noise );
	t->SetBranchAddress( "pedvar", &pedvar );
	t->SetBranchAddress( "nbins", &t_nbins );
	t->SetBranchAddress( "e0", t_e0 );
	t->SetBranchAddress( "eff", t_eff );
	
	// rate calculator
	VMonteCarloRateCalculator* fMCR = new VMonteCarloRateCalculator();
	// hardwired Whipple spectrum
	double fWhippleNorm = 3.250e-11;
	double fWhippleIndex = -2.440;
	
	// loop over all effective areas and calculate expected rates
	unsigned int iN = t->GetEntries();
	
	cout << "reading " << iN << " effective areas from ";
	cout << fEffAreaFile << endl;
	cout << endl;
	for( unsigned int i = 0; i < iN; i++ )
	{
		t->GetEntry( i );
		
		az     = ( int )t_az;
		noise  = ( int )t_noise;
		
		vector< double > fenergy;
		vector< double > feffectivearea;
		for( int e = 0; e < t_nbins; e++ )
		{
			fenergy.push_back( t_e0[e] );
			feffectivearea.push_back( t_eff[e] );
		}
		
		MCrate = fMCR->getMonteCarloRate(
					 fenergy, feffectivearea,
					 fWhippleIndex, fWhippleNorm, 1., fEnergyThreshold, 1.e7, false );
		fMC->Fill();
		
		for( int e = 1; e <= h->GetXaxis()->GetNbins(); e++ )
		{
			fill_profilehistogram_for_TMVA(
				h,
				h->GetXaxis()->GetBinCenter( e ),
				ze,
				fMCR->getMonteCarloRate(
					fenergy, feffectivearea,
					fWhippleIndex, fWhippleNorm, 1.,
					TMath::Power( 10., h->GetXaxis()->GetBinLowEdge( e ) ),
					TMath::Power( 10., h->GetXaxis()->GetBinUpEdge( e ) ),
					false ) );
		}
	}
	f->Close();
	return fMC;
}

/*
 * read list of runs
 *
*/
vector< string > read_run_list( string iRunList )
{
	vector< string > i_runs;
	ifstream is( iRunList.c_str(), ifstream::in );
	if( !is )
	{
		cout << "Error reading energy bins from list of background files files" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	string is_line;
	while( getline( is, is_line ) )
	{
		i_runs.push_back( is_line );
	}
	cout << "Found list with " << i_runs.size() << " background runs" << endl;
	
	return i_runs;
}

/*
 * fill background rates for a given run
 *
 * assume that anasum files contain a single run only (!!)
 */
void fillBackgroundRates_perRun( string i_runFileName, TProfile2D* h )
{
	TFile* f = new TFile( i_runFileName.c_str() );
	if( f->IsZombie() )
	{
		cout << "Background rate file not found: " << i_runFileName << endl;
		return;
	}
	TTree* t = ( TTree* )f->Get( "total_1/stereo/tRunSummary" );
	if( !t )
	{
		cout << "Background rate file incomplete (missing tRunSummary tree): " << i_runFileName << endl;
		return;
	}
	int runOn = 0;
	double elevationOff = 0.;
	double OffNorm = 0.;
	double NOff = 0.;
	double tOff = 0.;
	t->SetBranchAddress( "runOn", &runOn );
	t->SetBranchAddress( "elevationOff", &elevationOff );
	t->SetBranchAddress( "OffNorm", &OffNorm );
	t->SetBranchAddress( "tOff", &tOff );
	t->SetBranchAddress( "NOff", &NOff );
	t->GetEntry( 0 );
	// skip runs shorter than 10 min
	if( tOff < 600. || NOff < 0.01 )
	{
		cout << "\t skipping short run " << runOn << " (run length " << tOff << "s)" << endl;
		f->Close();
		return;
	}
	cout << "RUN " << elevationOff << ", " << tOff << ", " << OffNorm << ", " << elevationOff << ", " << NOff << ", " << runOn << endl;
	stringstream iTemp;
	iTemp << "run_" << runOn << "/stereo/energyHistograms/herecCounts_off";
	TH1D* hOff = ( TH1D* )f->Get( iTemp.str().c_str() );
	if( !hOff )
	{
		cout << "Background rate file incomplete (missing off histogram)" << i_runFileName << endl;
		return;
	}
	for( int e = 1; e <= h->GetXaxis()->GetNbins(); e++ )
	{
		int i_binStart = hOff->GetXaxis()->FindBin( h->GetXaxis()->GetBinLowEdge( e ) );
		int i_binStop = hOff->GetXaxis()->FindBin( h->GetXaxis()->GetBinUpEdge( e ) );
		double iR = 0.;
		for( int b = i_binStart; b <= i_binStop; b++ )
		{
			iR += hOff->GetBinContent( b );
		}
		if( tOff > 0. && iR > 0. )
		{
			h->Fill(
				h->GetXaxis()->GetBinCenter( e ),
				90. - elevationOff,
				iR * OffNorm / tOff * 60. );
		}
	}
	
	f->Close();
}


/*
 * fill background rates from anasum files
 *
 */
void fillBackgroundRates( string iRunList, TProfile2D* h )
{
	vector< string > i_runs =  read_run_list( iRunList );
	
	for( unsigned int i = 0; i < i_runs.size(); i++ )
	{
		fillBackgroundRates_perRun( i_runs[i], h );
	}
}

/*
 * convert 2D histograms to 2D graphs
 *
 */
TGraph2DErrors* getTGraph2D( vector< TProfile2D* > h, string iGraphName )
{
	TGraph2DErrors* i_g = new TGraph2DErrors( 1 );
	i_g->SetName( iGraphName.c_str() );
	if( iGraphName == "gONRate" )
	{
		i_g->SetTitle( "signal + background rate (1/s)" );
	}
	else
	{
		i_g->SetTitle( "background rate (1/s)" );
	}
	unsigned int z = 0;
	for( int b_x = 1; b_x <= h[1]->GetNbinsX(); b_x++ )
	{
		for( int b_y = 1; b_y <= h[1]->GetNbinsY(); b_y++ )
		{
			double i_z = h[1]->GetBinContent( b_x, b_y );
			if( iGraphName == "gONRate" )
			{
				i_z += h[0]->GetBinContent( b_x, b_y );
			}
			// expect rate graphs in 1./s
			i_g->SetPoint(
				z,
				h[1]->GetXaxis()->GetBinCenter( b_x ),
				h[1]->GetYaxis()->GetBinCenter( b_y ),
				i_z / 60. );
			i_g->SetPointError( z, 0., 0., 0. );
			z++;
		}
	}
	return i_g;
}


int main( int argc, char* argv[] )
{
	cout << endl;
	cout << "calculateCrabRateFromMC (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
	cout << "--------------------------------" << endl;
	if( argc != 6 )
	{
		cout << "VTS.calculateCrabRateFromMC <effective area file> <outputfile> ";
		cout << "<energy threshold [TeV]> ";
		cout << "<TMVA parameter file> ";
		cout << "<run list for anasum background files>";
		cout << endl << endl;
		exit( EXIT_SUCCESS );
	}
	cout << endl;
	
	string fEffAreaFile = argv[1];
	string ioffile = argv[2];
	// energy threshold
	double fEnergyThreshold = atof( argv[3] );
	cout << "energy threshold: " << fEnergyThreshold << " TeV" << endl;
	if( fEnergyThreshold < 1.e-2 )
	{
		fEnergyThreshold = 1.e-2;    // take care of log(fEnergyThreshold)
	}
	string fTMVAParameterFile = argv[4];
	string fBackgroundRunList = argv[5];
	
	cout << "Note! Hardwired Whipple spectrum (power law)" << endl;
	
	TFile* f1 = new TFile( ioffile.c_str(), "RECREATE" );
	if( f1->IsZombie() )
	{
		cout << "error opening output file: " << ioffile << endl;
		exit( EXIT_SUCCESS );
	}
	
	vector< TProfile2D* > hRateProfileHisto = initializeRateProfileHistos(
				fEffAreaFile, fTMVAParameterFile );
				
	TTree* fMC = fillMCRates(
					 fEffAreaFile, hRateProfileHisto[0], fEnergyThreshold );
					 
	fillBackgroundRates(
		fBackgroundRunList, hRateProfileHisto[1] );
		
	cout << "writing results to " << f1->GetName() << endl;
	f1->cd();
	fMC->Write();
	for( unsigned int i = 0; i < hRateProfileHisto.size(); i++ )
	{
		hRateProfileHisto[i]->Write();
	}
	getTGraph2D( hRateProfileHisto, "gONRate" )->Write();
	getTGraph2D( hRateProfileHisto, "gBGRate" )->Write();
	f1->Close();
}
