/* \file writeVTSWPPhysSensitivityFiles write VTS sensitivity files (CTA style)

   write a simple root file with histograms for sensitivities, effective areas,
   angular and energy resolution, migration matrix, etc.

   follow here CTA WP-Phys standards (with minor modifications)

   PRELIMINARY: many hardcoded values

   \author Gernot Maier

*/

#include "VGammaHadronCuts.h"
#include "VWPPhysSensitivityFile.h"

#include "TFile.h"
#include "TH1F.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main( int argc, char* argv[] )
{
	/////////////////////
	// input parameters
	if( argc != 4 )
	{
		cout << endl;
		cout << "./writeVTSWPPhysSensitivityFiles <gamma effective area file> <anasum file (Crab Nebula)> <output root file>" << endl;
		cout << endl;
		exit( 0 );
	}
	string fEffectiveAreaFile = argv[1];
	string fAnaSumFile = argv[2];
	string fOutputFile = argv[3];
	string fObservatory = "V6";
	
	cout << endl;
	cout << "writing VTS sensitivity file" << endl;
	cout << endl;
	cout << "\t effective area file:\t" << fEffectiveAreaFile << endl;
	cout << "\t anasum file:\t" << fAnaSumFile;
	cout << "\t output file:\t" << fOutputFile << endl;
	cout << endl;
	
	/////////////////////
	// initialization
	VWPPhysSensitivityFile* iData = new VWPPhysSensitivityFile();
	//    iData->setDebug( true );
	iData->setObservatory( fObservatory );
	// output file
	if( !iData->initializeOutputFile( fOutputFile ) )
	{
		exit( EXIT_FAILURE );
	}
	// Crab spectrum from HEGRA (CTA default)
	iData->setCrabSpectrum( "$VERITAS_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat", 5 );
	// CR spectra (protons + electrons)
	iData->setCosmicRaySpectrum( "$VERITAS_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CR.dat", 0, 8 );
	
	/////////////////////////////////////
	// on source histograms
	// initialize histogram with the standard binning used in the VTS WP Phys group
	iData->initializeHistograms( 21, -1.9, 2.3, 500, -1.9, 2.3, 400, -2.3, 2.7, 9999 );
	
	if( !iData->fillIRFHistograms( fEffectiveAreaFile ) );
	{
		cout << "error filling on source histograms" << endl;
	}
	
	iData->terminate();
	cout << "end of calculating sensitivities" << endl;

        ////////////////////////////////////////
        // read anasum file for background rates
        ////////////////////////////////////////

        cout << endl;
        cout << "reading anasum file " << fAnaSumFile << endl;
        cout << endl;
        VEnergySpectrum iE( fAnaSumFile );
        if( iE.isZombie() )
        {
           cout << "error opening file " << fAnaSumFile << endl;
           exit( EXIT_FAILURE );
        }
        iE.setEnergyBinning( 0.05 );
        iE.combineRuns();
        TH1D *hBckCounts = (TH1D*)iE.getEnergyCountingOffHistogram();
        TH1D *hObsTime   = (TH1D*)iE.getTotalTimeHistogram( true );
        if( hBckCounts && hObsTime )
        {
            cout << hBckCounts->GetEntries() << endl;
            TFile iF( fOutputFile.c_str(), "update" );
            ////////////////////////////////
            // background rates
            TH1F *hBckRate = new TH1F( "BGRate", "", hBckCounts->GetNbinsX(), hBckCounts->GetXaxis()->GetXmin(), hBckCounts->GetXaxis()->GetXmax() );
            hBckRate->Sumw2();
            hBckRate->SetXTitle( hBckCounts->GetXaxis()->GetTitle() );
            hBckRate->SetYTitle( "background rate [1/s]" );
            hBckRate->Divide( hBckCounts, hObsTime );
            hBckRate->Scale( 60. );
            ////////////////////////////////
            // backgrounds per square degrees
            TH1F *hBckRateDeq = (TH1F*)hBckRate->Clone( "BGRatePerSqDeg" );
            hBckRateDeq->SetYTitle( "background rate [1/s/deg^{2}]" );
            
            // read gamma/hadron cuts from anasum file
            TFile iFile_anasum( fAnaSumFile.c_str() );
            if( !iFile_anasum.IsZombie() )
            {
                VGammaHadronCuts *i_GammaHadronCuts = (VGammaHadronCuts*)iFile_anasum.Get( "GammaHadronCuts" );
                if( i_GammaHadronCuts )
                {
                    for( int b = 1; b <= hBckRateDeq->GetNbinsX(); b++ )
                    {
                        double i_t2_max = i_GammaHadronCuts->getTheta2Cut_max( TMath::Power( 10., hBckRateDeq->GetBinCenter( b ) ) );
                        double i_t2_min = i_GammaHadronCuts->getTheta2Cut_min( TMath::Power( 10., hBckRateDeq->GetBinCenter( b ) ) );
                        double i_omega =  2. * TMath::Pi() * ( 1. - cos( sqrt( i_t2_max ) * TMath::Pi() / 180. ) );
                        if( i_t2_min > 0. ) i_omega -= 2. * TMath::Pi() * ( 1. - cos( sqrt( i_t2_min ) * TMath::Pi() / 180. ) );
                        i_omega *= TMath::RadToDeg() * TMath::RadToDeg();     // sr to deg^2
                        cout << "\t " << b << "\t" << i_t2_min << "\t" << i_t2_max << "\t" << i_omega << endl;
                        if( i_omega > 0. )
                        {
                            hBckRateDeq->SetBinContent( b, hBckRateDeq->GetBinContent( b ) / i_omega );
                            hBckRateDeq->SetBinError( b, hBckRateDeq->GetBinError( b ) / i_omega );
                        }
                    }
                }
                else
                {
                   cout << "error: gamma/hadron cuts not found in anasum file" << endl;
                }
            }
            iFile_anasum.Close();
            if( !iF.IsZombie() && iF.cd() )
            {
               hBckRate->Write();
               if( hBckRateDeq ) hBckRateDeq->Write();
            }
            iF.Close();
        }
      
}
