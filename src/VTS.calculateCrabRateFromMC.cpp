/*! \file VTS.calculateCrabRateFromMC.cpp
    \brief calculate gamma ray rate from Crab Nebula with effective areas from MC



*/

#include "TF1.h"
#include "TFile.h"
#include "TTree.h"

#include "CEffArea.h"
#include "VGlobalRunParameter.h"
#include "VMonteCarloRateCalculator.h"
#include "VEnergySpectrumfromLiterature.h"

#include <iostream>
#include <vector>


using namespace std;

int main( int argc, char* argv[] )
{
	bool bDebug = false;
	
	cout << endl;
	cout << "VTS.calculateCrabRateFromMC (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
	cout << "--------------------------------" << endl;
	if( argc != 5 )
	{
		cout << "VTS.calculateCrabRateFromMC <effective area file> <outputfile> ";
		cout << "<energy threshold [TeV]> <dead time (0=no dead time)" << endl;
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
	bool bUsePowerLawOnly = true;
	if( bUsePowerLawOnly )
	{
		cout << "Note! Hardwired Whipple spectrum (power law)" << endl;
	}
	
	float ze = 0.;
	int az = 0;
	float Woff = 0.;
	int noise = 0;
	float pedvar = 0.;
	unsigned int nrates = 0;
	float MCrate[1000];
	
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
	fMC->Branch( "nrates", &nrates, "nrates/i" );
	fMC->Branch( "MCrate", MCrate, "MCrate[nrates]/F" );
	
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
	
	char hname[2000];
	vector< unsigned int > fID;
	vector< VEnergySpectrumfromLiterature* > fESpecFun;
	// Crab Nebula Spectra
	if( !bUsePowerLawOnly )
	{
		fID.push_back( 1 );                           // Whipple
		fID.push_back( 5 );                           // HEGRA
		fID.push_back( 2 );                           // H.E.S.S.
		fID.push_back( 5 );                           // HEGRA
		fID.push_back( 6 );                           // MAGIC
		VGlobalRunParameter* fRunParameter = new VGlobalRunParameter();
		sprintf( hname,
				 "%s/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat",
				 fRunParameter->getDirectory_EVNDISPAnaData().c_str() );
		VEnergySpectrumfromLiterature* fESpec = new VEnergySpectrumfromLiterature( hname );
		if( fESpec->isZombie() )
		{
			cout << "Error: file with energy spectra from literature not found: " << hname << endl;
			exit( EXIT_FAILURE );
		}
		for( unsigned int i = 0; i < fID.size(); i++ )
		{
			fESpecFun.push_back( fESpec );
		}
		
		// error checks
		if( fESpecFun.size() != fID.size() )
		{
			cout << "Error: vector mismatch - check code" << endl;
			exit( EXIT_FAILURE );
		}
		
		// print everything
		for( unsigned int i = 0; i < fESpecFun.size(); i++ )
		{
			if( fESpecFun[i] )
			{
				fESpecFun[i]->listValues( fID[i] );
			}
		}
	}
	
	// rate calculator
	VMonteCarloRateCalculator* fMCR = new VMonteCarloRateCalculator();
	
	// loop over all effective areas and calculate expected rates
	unsigned int iN = t->GetEntries();
	cout << endl;
	cout << "reading " << iN << " effective areas from ";
	cout << hname << endl;
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
		
		
		if( bUsePowerLawOnly )
		{
			// hardwired Whipple spectrum (much faster than outer function call)
			nrates = 1;
			MCrate[0] = fMCR->getMonteCarloRate( fenergy, feffectivearea, -2.440, 3.250e-11, 1., fEnergyThreshold, 1.e7, bDebug );
		}
		else
		{
			nrates = fESpecFun.size();
			for( unsigned int t = 0; t < fESpecFun.size(); t++ )
			{
				// TODO fix call
				//                        MCrate[t] = fMCR->getMonteCarloRate(
				//                                fenergy, feffectivearea, fESpecFun[t], fID[t], fEnergyThreshold, 1.e7, bDebug );
				if( bDebug )
				{
					cout << "MC Rate " << ze << "\t" << MCrate[t] << endl;
				}
			}
		}
		fMC->Fill();
	}
	cout << endl;
	cout << "writing results to " << f1->GetName() << endl;
	f1->cd();
	fMC->Write();
	f1->Close();
}
