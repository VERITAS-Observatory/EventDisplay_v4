/*! \file VTS.calculateCrabRateFromMC.cpp
    \brief calculate gamma ray rate from Crab Nebula with effective areas from MC



*/

#include "TF1.h"
#include "TFile.h"
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
	if( fMCRates_TMVA )
	{
		fMCRates_TMVA->Fill( i_E, i_Ze, iRate );
	}
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
	
	return tmp_e;
}


int main( int argc, char* argv[] )
{
	bool bDebug = false;
	
	cout << endl;
	cout << "VTS.calculateCrabRateFromMC (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
	cout << "--------------------------------" << endl;
	if( argc != 6 )
	{
		cout << "VTS.calculateCrabRateFromMC <effective area file> <outputfile> ";
		cout << "<energy threshold [TeV]> <dead time (0=no dead time) ";
		cout << "<TMVA parameter file>" << endl;
		cout << endl;
		exit( EXIT_SUCCESS );
	}
	cout << endl;
	
	string ieff = argv[1];
	string ioffile = argv[2];
	// energy threshold
	double fEnergyThreshold = atof( argv[3] );
	cout << "energy threshold: " << fEnergyThreshold << " TeV" << endl;
	if( fEnergyThreshold < 1.e-2 )
	{
		fEnergyThreshold = 1.e-2;    // take care of log(fEnergyThreshold)
	}
	double fDeadTime = atof( argv[4] );
	cout << "dead time: " << fDeadTime << endl;
	
	cout << "Note! Hardwired Whipple spectrum (power law)" << endl;
	
	float ze = 0.;
	int az = 0;
	float Woff = 0.;
	int noise = 0;
	float pedvar = 0.;
	float MCrate;
	
	TFile* f1 = new TFile( ioffile.c_str(), "RECREATE" );
	if( f1->IsZombie() )
	{
		cout << "error opening output file: " << ioffile << endl;
		exit( EXIT_SUCCESS );
	}
	TTree* fMC = new TTree( "fMCRate", "MC rate predictions" );
	fMC->Branch( "ze", &ze, "ze/F" );
	fMC->Branch( "az", &az, "az/I" );
	fMC->Branch( "Woff", &Woff, "Woff/F" );
	fMC->Branch( "noise", &noise, "noise/I" );
	fMC->Branch( "pedvar", &pedvar, "pedvar/F" );
	fMC->Branch( "MCrate", &MCrate, "MCrate/F" );
	
	TFile* f = new TFile( ieff.c_str() );
	if( f->IsZombie() )
	{
		cout << "error opening file with effective areas " << ieff << endl;
		exit( EXIT_FAILURE );
	}
	TTree* t = ( TTree* )gDirectory->Get( "fEffAreaH2F" );
	if( !t )
	{
		cout << "error reading effective area tree from " << ieff << endl;
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
	
	unsigned int iN = t->GetEntries();
	// profile histogram of MC rates as function of energy and zenith angle
	vector< double > tmp_ebins = read_energy_bins( argv[5] );
	cout << "Energy bins: ";
	for( unsigned int e = 0; e < tmp_ebins.size(); e++ )
	{
		cout << tmp_ebins[e] << ", ";
	}
	cout << endl;
	// - read unique vector of zenith angles
	vector< double > tmp_zebins;
	for( unsigned int i = 0; i < iN; i++ )
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
	double* ie = &tmp_ebins[0];
	double* iz = &tmp_zebin_edges[0];
	TProfile2D* fMCRates_TMVA = new TProfile2D(
		"MCRatesTMVA", "",
		( int )tmp_ebins.size() - 1, ie,
		( int )tmp_zebin_edges.size() - 1, iz );
	fMCRates_TMVA->SetXTitle( "log_{10} Energy/TeV" );
	fMCRates_TMVA->SetYTitle( "zenith angle (deg)" );
	fMCRates_TMVA->SetZTitle( "rate from Crab Nebula (1/min)" );
	
	// rate calculator
	VMonteCarloRateCalculator* fMCR = new VMonteCarloRateCalculator();
	
	// loop over all effective areas and calculate expected rates
	cout << endl;
	cout << "reading " << iN << " effective areas from ";
	cout << ieff << endl;
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
		
		// hardwired Whipple spectrum
		MCrate = fMCR->getMonteCarloRate(
					 fenergy, feffectivearea,
					 -2.440, 3.250e-11, 1., fEnergyThreshold, 1.e7, bDebug );
		fMC->Fill();
		
		// fill profile histogram for TMVA
		for( unsigned int e = 0; e < tmp_ebins.size() - 1; e++ )
		{
			fill_profilehistogram_for_TMVA(
				fMCRates_TMVA,
				0.5 * ( tmp_ebins[e + 1] + tmp_ebins[e] ), ze,
				fMCR->getMonteCarloRate(
					fenergy, feffectivearea,
					-2.440, 3.250e-11, 1.,
					TMath::Power( 10., tmp_ebins[e] ),
					TMath::Power( 10., tmp_ebins[e + 1] ),
					bDebug ) );
		}
	}
	cout << endl;
	cout << "writing results to " << f1->GetName() << endl;
	f1->cd();
	fMC->Write();
	fMCRates_TMVA->Write();
	f1->Close();
}
