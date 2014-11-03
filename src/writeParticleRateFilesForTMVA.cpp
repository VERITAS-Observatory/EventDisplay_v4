/* writeParticleRateFilesForTMVA

   write files with particle number spectra for on (gamma) and off (protons+electrons) counts

   files are needed e.g. for setting the optimal cut value for TMVA cuts

   Input:
       * Combined anasum files for each zenith angle bin

   Output:
       * root file with signal and background rates per zenith angle and energy bi

*/

#include "VTMVARunData.h"
#include "VInstrumentResponseFunctionReader.h"
#include "VWPPhysSensitivityFile.h"

#include <iostream>
#include <string>
#include <vector>

#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH2D.h"
#include "TGraph2D.h"
#include "TMath.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TGraph2D.h"
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
		//cout << " [qsub options]" << endl;
		//cout << argc << endl;
		cout << endl;
		exit( 0 );
	}
	
	cout << endl;
	cout << "writeParticleRateFilesForTMVA" << endl;
	cout << "========================================" << endl;
	cout << endl ;
	
	string iDataDir = argv[1];
	string iOutFil = argv[2];
	
	Int_t runOn;
	Double_t OffNorm, elevationOn, elevationOff, azimuthOn, azimuthOff;
	Double_t tOn = 0.;
	Double_t tObs, alphaNorm;
	tObs = 0.;
	alphaNorm = 0.;
	
	/* Name and directory to the COMBINED anasum root file */
	char iInputFile[800];
	sprintf( iInputFile, "%s", iDataDir.c_str() );
	TFile* f1 = new TFile( iInputFile, "OPEN" );
	
	char reddataname[256], histnameOn[256], histnameOff[256];
	TTree* RunSum = ( TTree* )f1->Get( "total_1/stereo/tRunSummary" );
	
	RunSum->SetBranchAddress( "runOn", &runOn );
	RunSum->SetBranchAddress( "runOn", &runOn );
	RunSum->SetBranchAddress( "OffNorm", &OffNorm );
	RunSum->SetBranchAddress( "elevationOn", &elevationOn );
	RunSum->SetBranchAddress( "elevationOff", &elevationOff );
	RunSum->SetBranchAddress( "azimuthOn", &azimuthOn );
	RunSum->SetBranchAddress( "azimuthOff", &azimuthOff );
	RunSum->SetBranchAddress( "tOn", &tOn );
	
	double iZenithperrun;
	
	double new_on;
	double new_off;
	double new_on_error, new_off_error;
	
	unsigned int iBin = 0;
	
	for( unsigned int i = 0; i < ( RunSum->GetEntries() - 1 ); i++ )
	{
		RunSum->GetEntry( i );
		
		sprintf( histnameOn, "run_%d/stereo/energyHistograms/herecCounts_on", runOn );
		sprintf( histnameOff, "run_%d/stereo/energyHistograms/herecCounts_off", runOn );
		TH1D* t2 = ( TH1D* )f1->Get( histnameOff );
		
		//unsigned int nbins_t1 = t1->GetNbinsX();
		unsigned int nbins_t2 = t2->GetNbinsX();
		iBin = nbins_t2;
	}
	
	// -----------------------------------------------------------------
	//
	// loop over the number of runs in the combined anasum root file
	//
	// check if the zenith angle is within a range of 5 degrees (zenith angle intervals are used for an easier optimization)
	//
	// -----------------------------------------------------------------
	
	TGraph2DErrors* tRatePerEnergyON  = new TGraph2DErrors();
	TGraph2DErrors* tRatePerEnergyOFF = new TGraph2DErrors();
	TGraph2DErrors* tRatePerEnergySignal = new TGraph2DErrors();
	
	int l = 0;
	
	int m = 0;
	for( unsigned int i = 0; i < ( RunSum->GetEntries() - 1 ); i++ )
	{
	
		RunSum->GetEntry( i );
		alphaNorm = OffNorm;
		//iZenithperrun = 90.0 - elevationOn;
                iZenithperrun = 90.0 - elevationOff; 	        
		sprintf( histnameOn, "run_%d/stereo/energyHistograms/herecCounts_on", runOn );
		sprintf( histnameOff, "run_%d/stereo/energyHistograms/herecCounts_off", runOn );
		TH1D* t1 = ( TH1D* )f1->Get( histnameOn );
		TH1D* t2 = ( TH1D* )f1->Get( histnameOff );
		
		// get the observation time of each run
		sprintf( reddataname, "run_%d/stereo/pointingDataReduced", runOn );
		TTree* tReddata = ( TTree* )f1->Get( reddataname );
		
		tObs = tReddata->GetEntries();
		
		cout << "Run Number: " << runOn ;
		cout << ", Elevation angle = " << 90 - iZenithperrun ;
		cout << ", Zenith angle = " << iZenithperrun ;
		cout << ", Time for Run = " << tObs << "s" << endl;
		
		// get the bin content of the ON and OFF events per second and scale the off-counts with the alpha factor
		for( unsigned int k = 0; k < iBin; k++ )
		{
			new_on        = ( t1->GetBinContent( k ) ) * 1.0 / tObs;
			new_on_error  = ( t1->GetBinError( k ) ) * 1.0 / tObs;
			new_off       = ( t2->GetBinContent( k ) ) * alphaNorm / tObs;
			new_off_error = ( t2->GetBinError( k ) ) * alphaNorm / tObs;
			
			/* ON rate (signal and background) */
			tRatePerEnergyON->SetPoint( m, t1->GetBinCenter( k ), iZenithperrun, new_on );
			tRatePerEnergyON->SetPointError( m, 0., 0., new_on_error );
			tRatePerEnergyON->SetMarkerStyle( 20 );
			
			/* Signal rate (ON minus OFF) */
			tRatePerEnergySignal->SetPoint( m, t1->GetBinCenter( k ), iZenithperrun, TMath::Abs( new_on - new_off ) );
			tRatePerEnergySignal->SetPointError( m, 0., 0., ( TMath::Abs( new_on_error + new_off_error ) / 2.0 ) );
			tRatePerEnergySignal->SetMarkerStyle( 20 );
			
			/* OFF rate */
			tRatePerEnergyOFF->SetPoint( m, t1->GetBinCenter( k ), iZenithperrun, new_off );
			tRatePerEnergyOFF->SetPointError( m, 0., 0., new_off_error );
			tRatePerEnergyOFF->SetMarkerStyle( 20 );
			
			m++;
		}
		l++;
	}
	
	// -----------------------------------------------------------------
	//
	// Plot the ON, OFF and Signal rates (ON minus OFF) for the specific zenith angle intervals
	//
	// -----------------------------------------------------------------
	gStyle->SetPalette( 1 );
	
	/* ON rate = Signal plus background */
	TCanvas* c1 = new TCanvas( "c1", "1D_On_Counts", 0, 0, 700, 500 );
	c1->cd();
	c1->SetRightMargin( 0.15 );
	
	tRatePerEnergyON->SetTitle( "signal+background rate (1/sec); Log_{10}E [TeV]; Zenith angle [deg]" );
	tRatePerEnergyON->Draw( "P" );
	c1->Update();
	
	// OFF rate = Background rate
	TCanvas* c2 = new TCanvas( "c2", "1D_Off_Counts", 750, 0, 700, 500 );
	c2->cd();
	c2->SetRightMargin( 0.15 );
	
	tRatePerEnergyOFF->SetTitle( "background rate (1/sec); Log_{10}E [TeV]; Zenith angle [deg]" );
	tRatePerEnergyOFF->Draw( "P" );
	c2->Update();
	
	TCanvas* c3 = new TCanvas( "c3", "1D_Signal_Counts", 750, 750, 700, 500 );
	c3->cd();
	c3->SetRightMargin( 0.15 );
	tRatePerEnergySignal->SetTitle( "signal rate (1/sec); Log_{10}E [TeV]; Zenith angle [deg] " );
	tRatePerEnergySignal->Draw( "P" );
	c3->Update();
	
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/* write particle number file */
	
	char iOutputFile[800];
	sprintf( iOutputFile, "%s", iOutFil.c_str() );
	TFile* SignalRateFile = new TFile( iOutputFile , "RECREATE" );
	
	SignalRateFile->cd();
	SignalRateFile->Write();
	
	// write graphs of ON rate to file
	tRatePerEnergyON->Write( "gONRate" );
	
	// write graphs of OFF rate to file
	tRatePerEnergyOFF->Write( "gBGRate" );
	
	// write graphs of signal rate to file
	tRatePerEnergySignal->Write( "gSignalRate" );
	
	return 0;
}
