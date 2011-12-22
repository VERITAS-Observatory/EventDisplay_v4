/* \file writeCTAWPPhysSensitivityFiles write CTA WP Phys sensitivity files

*/

#include "VWPPhysSensitivityFile.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main( int argc, char *argv[] )
{
    if( argc != 7 )
    {
        cout << endl;
	cout << "./writeCTAWPPhysSensitivityFiles <sub array> <observing time> <data directory> <outputfile> <observatory (CTA/V5/V6)> <offset=0/1>" << endl;
	cout << endl;
	exit( 0 );
    }
    string fSubArray = argv[1];
    double fObservingTime_h = atof( argv[2] );
    string fDataDirectory = argv[3];
    string fOutputFile = argv[4];
    string fObservatory = argv[5];
    bool   fWriteOffsetFiles = atoi( argv[6] );

    VWPPhysSensitivityFile *iData = new VWPPhysSensitivityFile();
    iData->setObservatory( fObservatory );
// Note: offset binning hardwired
    vector< double > iWobbleMin;
    vector< double > iWobbleMax;
    if( fWriteOffsetFiles )
    {
       iWobbleMin.push_back( 0.0 ); iWobbleMax.push_back( 1. );
       iWobbleMin.push_back( 1.0 ); iWobbleMax.push_back( 2.0 );
       iWobbleMin.push_back( 2.0 ); iWobbleMax.push_back( 3.0 );
       iWobbleMin.push_back( 3.0 ); iWobbleMax.push_back( 3.5 );
       iWobbleMin.push_back( 3.5 ); iWobbleMax.push_back( 4.0 );
       iWobbleMin.push_back( 4.0 ); iWobbleMax.push_back( 4.5 );
       iWobbleMin.push_back( 4.5 ); iWobbleMax.push_back( 5.0 );
//       iWobbleMin.push_back( 5.0 ); iWobbleMax.push_back( 5.5 ); 
//       iWobbleMin.push_back( 5.5 ); iWobbleMax.push_back( 6.0 );
    }
// sub array
    iData->setSubArray( fSubArray );
// observing time
    iData->setObservationTime( fObservingTime_h );
// output file
    if( !iData->initializeOutputFile( fOutputFile ) ) exit( -1 );
// default Crab spectrum from HEGRA
    iData->setCrabSpectrum( "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat", 5 );
// CR spectra
    iData->setCosmicRaySpectrum( "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CR.dat", 0, 2 );

// on source histograms
    iData->initializeHistograms( 20, -1.8, 2.2, 500, -1.8, 2.2, 400, -2.3, 2.7, 9999 );
    if( !iData->fillHistograms1D( fDataDirectory ) )
    {
       exit( -1 );
    } 

// off source histograms
    for( unsigned int i = 0; i < iWobbleMin.size(); i++ )
    {
       cout << "WOBBLE " << i << "\t" << iWobbleMin[i] << "\t" << iWobbleMax[i] << endl;
       iData->initializeHistograms( 20, -1.8, 2.2, 500, -1.8, 2.2, 400, -2.3, 2.7, i );
       if( !iData->fillHistograms1D( fDataDirectory ) )
       {
	  exit( -1 );
       }
    }
    if( iWobbleMin.size() > 0 ) iData->fillHistograms2D( iWobbleMin, iWobbleMax );

// 
    iData->terminate();
}
