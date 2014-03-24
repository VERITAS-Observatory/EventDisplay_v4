/* \file writeCTAWPPhysSensitivityFiles write CTA WP Phys sensitivity files

   write a simple root file with histograms for sensitivities, effective areas,
   angular and energy resolution, migration matrix, etc.

   follow here CTA WP-Phys standards (with minor modifications)

   \author Gernot Maier

*/

#include "VWPPhysSensitivityFile.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main( int argc, char* argv[] )
{
	/////////////////////
	// input parameters
	if( argc != 8 && argc != 7 )
	{
		cout << endl;
		cout << "./writeCTAWPPhysSensitivityFiles <sub array> <observing time> <data directory>";
		cout << " <outputfile> <observatory (CTA/V5/V6)> <calculate offset sensitivites=0/1> [recid (default=0)]" << endl;
		cout << endl;
		cout << "\t observing time: give unit without space, e.g. 50h, 10m, 2s" << endl;
		cout << endl;
		exit( 0 );
	}
	string fSubArray = argv[1];
	// observing time (translate from whatever unit into h)
	double fObservingTime_h = 0.;
	string iObstime = argv[2];
	if( iObstime.find( "h" ) != string::npos )
	{
		fObservingTime_h = atof( iObstime.substr( 0, iObstime.find( "h" ) ).c_str() );
	}
	else if( iObstime.find( "m" ) != string::npos )
	{
		fObservingTime_h = atof( iObstime.substr( 0, iObstime.find( "m" ) ).c_str() ) / 60.;
	}
	else if( iObstime.find( "s" ) != string::npos )
	{
		fObservingTime_h = atof( iObstime.substr( 0, iObstime.find( "s" ) ).c_str() ) / 3600.;
	}
	else
	{
		fObservingTime_h = atof( iObstime.c_str() );
	}
	string fDataDirectory = argv[3];
	string fOutputFile = argv[4];
	string fObservatory = argv[5];
	bool   fWriteOffsetFiles = atoi( argv[6] );
	int    fReconstructionID = 0;
	if( argc == 8 )
	{
		fReconstructionID = atoi( argv[7] );
	}
	
	/////////////////////
	// initialization
	VWPPhysSensitivityFile* iData = new VWPPhysSensitivityFile();
	iData->setDebug( true );
	iData->setObservatory( fObservatory );
	// Note: offset binning hardwired
	vector< double > iWobbleMin;
	vector< double > iWobbleMax;
	if( fWriteOffsetFiles )
	{
		iWobbleMin.push_back( 0.0 );
		iWobbleMax.push_back( 1. );
		iWobbleMin.push_back( 1.0 );
		iWobbleMax.push_back( 2.0 );
		iWobbleMin.push_back( 2.0 );
		iWobbleMax.push_back( 3.0 );
		iWobbleMin.push_back( 3.0 );
		iWobbleMax.push_back( 3.5 );
		iWobbleMin.push_back( 3.5 );
		iWobbleMax.push_back( 4.0 );
		iWobbleMin.push_back( 4.0 );
		iWobbleMax.push_back( 4.5 );
		iWobbleMin.push_back( 4.5 );
		iWobbleMax.push_back( 5.0 );
	}
	// sub array
	iData->setDataFiles( fSubArray, fReconstructionID );
	// observing time
	iData->setObservationTime( fObservingTime_h );
	// output file
	if( !iData->initializeOutputFile( fOutputFile ) )
	{
		exit( EXIT_FAILURE );
	}
	// Crab spectrum from HEGRA (CTA default)
	iData->setCrabSpectrum( "$CTA_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat", 5 );
	// CR spectra (protons + electrons)
	iData->setCosmicRaySpectrum( "$CTA_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CR.dat", 0, 8 );
	
	/////////////////////////////////////
	// on source histograms
	// initialize histogram with the standard binning used in the CTA WP Phys group
	iData->initializeHistograms( 21, -1.9, 2.3, 500, -1.9, 2.3, 400, -2.3, 2.7, 9999 );
	if( !iData->fillHistograms1D( fDataDirectory, true ) )
	{
		cout << "error filling on source histograms" << endl;
	}
	
	/////////////////////////////////////
	// off source histograms
	for( unsigned int i = 0; i < iWobbleMin.size(); i++ )
	{
		cout << "Camera offset: " << i << "\t" << iWobbleMin[i] << "\t" << iWobbleMax[i] << endl;
		// initialize histogram with the standard binning used in the CTA WP Phys group
		iData->initializeHistograms( 21, -1.9, 2.3, 500, -1.9, 2.3, 400, -2.3, 2.7, i );
		if( !iData->fillHistograms1D( fDataDirectory, false ) )
		{
			exit( EXIT_FAILURE );
		}
	}
	if( iWobbleMin.size() > 0 )
	{
		iData->fillHistograms2D( iWobbleMin, iWobbleMax );
	}
	
	iData->terminate();
	cout << "end of calculating sensitivities" << endl;
}
