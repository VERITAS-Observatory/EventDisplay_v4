/* \file writeVTSWPPhysSensitivityFiles write VTS sensitivity files (CTA style)

   write a simple root file with histograms for sensitivities, effective areas,
   angular and energy resolution, migration matrix, etc.

   follow here CTA WP-Phys standards (with minor modifications)

   PRELIMINARY: many hardcoded values

   # TODO;
   - calculate offaxis values
     - need to read in run parameters from a file?

   \author Gernot Maier

*/

#include "VAnalysisUtilities.h"
#include "VGammaHadronCuts.h"
#include "VWPPhysSensitivityFile.h"

#include "TFile.h"
#include "TH1F.h"

#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/////////////////////////
// global variables

// list of camera offsets and corresponding effective area file
map< double, string > fMCset;
// anasum file for background estimation
string fAnaSumFile;
// Observatory epoch
string fObservatory;

bool readRunParameterFile( string ifile )
{
	fMCset.clear();
	
	ifstream is;
	is.open( ifile.c_str(), ifstream::in );
	if( !is )
	{
		cout << "error opening run parameter file " << ifile << endl;
		return false;
	}
	string is_line;
	string temp;
	cout << endl;
	cout << "========================================" << endl;
	cout << "run parameter file(" << ifile << ")" << endl;
	
	while( getline( is, is_line ) )
	{
		if( is_line.size() > 0 )
		{
			istringstream is_stream( is_line );
			is_stream >> temp;
			if( temp != "*" )
			{
				continue;
			}
			cout << is_line << endl;
			is_stream >> temp;
			// MC sets <camera distance [deg]> <effective area file>
			if( temp == "MCset" )
			{
				is_stream >> temp;
				if( is_stream.eof() )
				{
					cout << "error while reading MCset line" << endl;
					return false;
				}
				is_stream >> fMCset[atof( temp.c_str() )];
			}
			// anasum file
			else if( temp == "ANASUMFILE" )
			{
				is_stream >> fAnaSumFile;
			}
			// observatory
			else if( temp == "OBSERVATORY" )
			{
				is_stream >> fObservatory;
			}
		}
	}
	is.close();
	
	return true;
}

int main( int argc, char* argv[] )
{
	/////////////////////
	// input parameters
	if( argc != 3 )
	{
		cout << endl;
		cout << "./writeVTSWPPhysSensitivityFiles <runparameter file> <output root file>" << endl;
		cout << endl;
		exit( 0 );
	}
	if( !readRunParameterFile( argv[1] ) )
	{
		return false;
	}
	string fOutputFile = argv[2];
	
	cout << endl;
	cout << "writing VTS sensitivity file (" << fObservatory << ")" << endl;
	cout << endl;
	unsigned int z = 0;
	for( map<double, string>::iterator it = fMCset.begin(); it != fMCset.end(); ++it )
	{
		cout << "\t offset " << it->first << " deg, ";
		cout << "effective area file: " << it->second;
		if( z == 0 )
		{
			cout << " (used for on-axis histograms)";
		}
		cout << endl;
		z++;
	}
	cout << "\t anasum file:\t" << fAnaSumFile << endl;
	cout << "\t output file:\t" << fOutputFile << endl;
	cout << endl;
	if( fMCset.size() == 0 )
	{
		cout << "error: no effective area files given" << endl;
		return false;
	}
	
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
	
	// first element is used for on source histograms
	if( !iData->fillIRFHistograms( fMCset.begin()->second, 20., fMCset.begin()->first ) )
	{
		cout << "error filling on source histograms" << endl;
	}
	// off-axis histograms
	z = 0;
	vector< double > i_woff;
	for( map<double, string>::iterator it = fMCset.begin(); it != fMCset.end(); ++it )
	{
		iData->initializeHistograms( 21, -1.9, 2.3, 500, -1.9, 2.3, 400, -2.3, 2.7, z );
		if( !iData->fillIRFHistograms( it->second, 20., it->first ) )
		{
			cout << "error filling off soure histograms: " << it->second << "\t" << it->first << endl;
		}
		i_woff.push_back( it->first );
		z++;
	}
	iData->fillHistograms2D( i_woff, i_woff );
	
	iData->terminate();
	
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
	TH1D* hBckCounts = ( TH1D* )iE.getEnergyCountingOffHistogram();
	TH1D* hObsTime   = ( TH1D* )iE.getTotalTimeHistogram( true );      // energy dependent observing time [s]
	if( hBckCounts && hObsTime )
	{
		TFile iF( fOutputFile.c_str(), "update" );
		////////////////////////////////
		// background rates
		TH1F* hBckRate = new TH1F( "BGRate", "", hBckCounts->GetNbinsX(), hBckCounts->GetXaxis()->GetXmin(), hBckCounts->GetXaxis()->GetXmax() );
		hBckRate->Sumw2();
		hBckRate->SetXTitle( hBckCounts->GetXaxis()->GetTitle() );
		hBckRate->SetYTitle( "background rate [1/s]" );
		hBckRate->Divide( hBckCounts, hObsTime );
		// scale by alpha
		VAnalysisUtilities i_ana;
		i_ana.openFile( fAnaSumFile, -1 );
		double i_alpha = i_ana.getNormalisationFactor();
		i_ana.closeFile();
		hBckRate->Scale( i_alpha );
		////////////////////////////////
		// backgrounds per square degrees
		TH1F* hBckRateDeq = ( TH1F* )hBckRate->Clone( "BGRatePerSqDeg" );
		hBckRateDeq->SetYTitle( "background rate [1/s/deg^{2}]" );
		
		// read gamma/hadron cuts from effective area file
		// note: assume same gamma/hadron cuts applied for effective area generation
		//       as for anasum analysis
		TFile iFile_effArea( fMCset.begin()->second.c_str() );
		if( !iFile_effArea.IsZombie() )
		{
			VGammaHadronCuts* i_GammaHadronCuts = ( VGammaHadronCuts* )iFile_effArea.Get( "GammaHadronCuts" );
			if( i_GammaHadronCuts )
			{
				cout << "found gamma/hadron cuts in effective area file" << endl;
				for( int b = 1; b <= hBckRateDeq->GetNbinsX(); b++ )
				{
					double i_t2_max = i_GammaHadronCuts->getTheta2Cut_max( TMath::Power( 10., hBckRateDeq->GetBinCenter( b ) ) );
					double i_t2_min = i_GammaHadronCuts->getTheta2Cut_min( TMath::Power( 10., hBckRateDeq->GetBinCenter( b ) ) );
					double i_omega =  2. * TMath::Pi() * ( 1. - cos( sqrt( i_t2_max ) * TMath::Pi() / 180. ) );
					if( i_t2_min > 0. )
					{
						i_omega -= 2. * TMath::Pi() * ( 1. - cos( sqrt( i_t2_min ) * TMath::Pi() / 180. ) );
					}
					if( i_omega > 0. )
					{
						hBckRateDeq->SetBinContent( b, hBckRateDeq->GetBinContent( b ) / ( i_omega * TMath::RadToDeg() * TMath::RadToDeg() ) );
						hBckRateDeq->SetBinError( b, hBckRateDeq->GetBinError( b ) / ( i_omega * TMath::RadToDeg() * TMath::RadToDeg() ) );
					}
				}
			}
			else
			{
				cout << "error: gamma/hadron cuts not found in anasum file" << endl;
			}
		}
		iFile_effArea.Close();
		if( !iF.IsZombie() && iF.cd() )
		{
			hBckRate->Write();
			if( hBckRateDeq )
			{
				hBckRateDeq->Write();
			}
		}
		iF.Close();
	}
	cout << endl << "all histograms written to " << fOutputFile << endl;
}
