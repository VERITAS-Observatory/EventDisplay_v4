/* writeParticleRateFilesForTMVA

   write files with particle number spectra for on (gamma) and off (protons+electrons) counts
   (differential counting rates)

   files are needed e.g. for setting the optimal cut value for TMVA cuts

   Input:
       * Combined anasum files for each zenith angle bin

   Output:
       * root file with signal and background rates per zenith angle and energy bi

*/


#include <iostream>
#include <string>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TGraph2DErrors.h"

using namespace std;

/*

    fill graphs with particle numbers for on (gamma) and off (protons+electrons) counts

    files are needed e.g. for setting the optimal cut value for TMVA cuts

*/
int main( int argc, char* argv[] )
{
	if( argc < 2 )
	{
		cout << endl;
		cout << "writeParticleRateFilesForTMVA <combined anasum root file> <outputfile>";
		cout << endl;
		exit( EXIT_FAILURE );
	}
	
	cout << endl;
	cout << "writeParticleRateFilesForTMVA" << endl;
	cout << "========================================" << endl;
	cout << endl ;
	
	string iDataDir = argv[1];
	string iOutFil = argv[2];
	
	/* Name and directory to the COMBINED anasum root file */
	TFile* f1 = new TFile( iDataDir.c_str() , "OPEN" );
	if( f1->IsZombie() )
	{
		cout << "Error reading file " << iDataDir << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	
	char reddataname[256], histnameOn[256], histnameOff[256];
	TTree* RunSum = ( TTree* )f1->Get( "total_1/stereo/tRunSummary" );
	if( !RunSum )
	{
		cout << "Error reading run summary tree" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	if( RunSum->GetEntries() == 0 )
	{
		cout << "Error reading run summary tree - empty tree" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	
	Int_t runOn = 0;
	Double_t OffNorm = 0.;
	Double_t elevationOff = 0.;
	Double_t  alphaNorm = 0.;
	RunSum->SetBranchAddress( "runOn", &runOn );
	RunSum->SetBranchAddress( "OffNorm", &OffNorm );
	RunSum->SetBranchAddress( "elevationOff", &elevationOff );
	
	// assume that all histograms for all runs have the same binning
	RunSum->GetEntry( 0 );
	sprintf( histnameOn, "run_%d/stereo/energyHistograms/herecCounts_on", runOn );
	sprintf( histnameOff, "run_%d/stereo/energyHistograms/herecCounts_off", runOn );
	TH1D* t2 = ( TH1D* )f1->Get( histnameOff );
	unsigned int iBin = t2->GetNbinsX();
	
	TGraph2DErrors* tRatePerEnergyON  = new TGraph2DErrors();
	tRatePerEnergyON->SetMarkerStyle( 20 );
	TGraph2DErrors* tRatePerEnergyOFF = new TGraph2DErrors();
	tRatePerEnergyOFF->SetMarkerStyle( 20 );
	TGraph2DErrors* tRatePerEnergySignal = new TGraph2DErrors();
	tRatePerEnergySignal->SetMarkerStyle( 20 );
	
	int m = 0;
	for( unsigned int i = 0; i < ( RunSum->GetEntries() - 1 ); i++ )
	{
	
		RunSum->GetEntry( i );
		alphaNorm = OffNorm;
		double iZenithperrun = 90.0 - elevationOff;
		sprintf( histnameOn, "run_%d/stereo/energyHistograms/herecCounts_on", runOn );
		sprintf( histnameOff, "run_%d/stereo/energyHistograms/herecCounts_off", runOn );
		TH1D* t1 = ( TH1D* )f1->Get( histnameOn );
		TH1D* t2 = ( TH1D* )f1->Get( histnameOff );
		
		// get the observation time of each run
		sprintf( reddataname, "run_%d/stereo/pointingDataReduced", runOn );
		TTree* tReddata = ( TTree* )f1->Get( reddataname );
		if( !tReddata )
		{
			cout << "Error reading pointingDataReduced tree for run " << runOn << endl;
			cout << "exiting..." << endl;
			exit( EXIT_FAILURE );
		}
		Double_t tObs = tReddata->GetEntries();
		
		cout << "Run Number: " << runOn ;
		cout << ", Zenith angle = " << iZenithperrun ;
		cout << ", Time for Run = " << tObs << "s" << endl;
		
		// get the bin content of the ON and OFF events per second and scale the off-counts with the alpha factor
		for( unsigned int k = 0; k < iBin; k++ )
		{
			double n_on        = ( t1->GetBinContent( k ) ) * 1.0 / tObs;
			double n_on_error  = ( t1->GetBinError( k ) ) * 1.0 / tObs;
			double n_off       = ( t2->GetBinContent( k ) ) * alphaNorm / tObs;
			double n_off_error = ( t2->GetBinError( k ) ) * alphaNorm / tObs;
			
			/* ON rate (signal and background) */
			tRatePerEnergyON->SetPoint( m, t1->GetBinCenter( k ), iZenithperrun, n_on );
			tRatePerEnergyON->SetPointError( m, 0., 0., n_on_error );
			
			/* Signal rate (ON minus OFF) */
			tRatePerEnergySignal->SetPoint( m, t1->GetBinCenter( k ), iZenithperrun, TMath::Abs( n_on - n_off ) );
			tRatePerEnergySignal->SetPointError( m, 0., 0., ( TMath::Abs( n_on_error + n_off_error ) / 2.0 ) );
			
			/* OFF rate */
			tRatePerEnergyOFF->SetPoint( m, t1->GetBinCenter( k ), iZenithperrun, n_off );
			tRatePerEnergyOFF->SetPointError( m, 0., 0., n_off_error );
			
			m++;
		}
	}
	
	// write particle number file
	TFile* SignalRateFile = new TFile( iOutFil.c_str() , "RECREATE" );
	
	// write graphs of ON rate to file
	tRatePerEnergyON->Write( "gONRate" );
	// write graphs of OFF rate to file
	tRatePerEnergyOFF->Write( "gBGRate" );
	// write graphs of signal rate to file
	tRatePerEnergySignal->Write( "gSignalRate" );
	SignalRateFile->Close();
	
	return 0;
}
