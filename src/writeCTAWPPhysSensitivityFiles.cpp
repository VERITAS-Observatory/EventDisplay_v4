/* \file writeCTAWPPhysSensitivityFiles write CTA WP Phys sensitivity files

   write a simple root files with histograms for sensitivities, effective areas,
   angular and energy resolution, migration matrix, etc.

   follow here CTA WP Phys standards 

*/

#include "VWPPhysSensitivityFile.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main( int argc, char *argv[] )
{
    if( argc != 8 )
    {
        cout << endl;
	cout << "./writeCTAWPPhysSensitivityFiles <sub array> <observing time [h]> <data directory>";
	cout << " <outputfile> <observatory (CTA/V5/V6)> <offset=0/1> <recid>" << endl;
	cout << endl;
	exit( 0 );
    }
    string fSubArray = argv[1];
    double fObservingTime_h = atof( argv[2] );
    string fDataDirectory = argv[3];
    string fOutputFile = argv[4];
    string fObservatory = argv[5];
    bool   fWriteOffsetFiles = atoi( argv[6] );
    int    fReconstructionID = atoi( argv[7] );

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
    }
// sub array
    iData->setDataFiles( fSubArray, fReconstructionID );
// observing time
    iData->setObservationTime( fObservingTime_h );
// output file
    if( !iData->initializeOutputFile( fOutputFile ) ) exit( -1 );
// default Crab spectrum from HEGRA
    iData->setCrabSpectrum( "$CTA_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat", 5 );
// CR spectra (protons + electrons)
    iData->setCosmicRaySpectrum( "$CTA_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CR.dat", 0, 2 );

// on source histograms
// initialize histogram with the standard binning used in the CTA WP Phys group
    iData->initializeHistograms( 20, -1.8, 2.2, 500, -1.8, 2.2, 400, -2.3, 2.7, 9999 );
    if( !iData->fillHistograms1D( fDataDirectory, true ) )
    {
       cout << "error filling on source histograms" << endl;
    }   

// off source histograms
    for( unsigned int i = 0; i < iWobbleMin.size(); i++ )
    {
       cout << "WOBBLE " << i << "\t" << iWobbleMin[i] << "\t" << iWobbleMax[i] << endl;
// initialize histogram with the standard binning used in the CTA WP Phys group
       iData->initializeHistograms( 20, -1.8, 2.2, 500, -1.8, 2.2, 400, -2.3, 2.7, i );
       if( !iData->fillHistograms1D( fDataDirectory, false ) )
       {
	  exit( -1 );
       }
    }
    if( iWobbleMin.size() > 0 ) iData->fillHistograms2D( iWobbleMin, iWobbleMax );

    iData->terminate();
    cout << "end of calculating sensitivities" << endl;
}
