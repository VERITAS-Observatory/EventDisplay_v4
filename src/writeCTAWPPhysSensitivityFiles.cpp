/* \file writeCTAWPPhysSensitivityFiles write CTA WP Phys sensitivity files

*/

#include "VWPPhysSensitivityFile.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main( int argc, char *argv[] )
{
    if( argc != 6 )
    {
        cout << endl;
	cout << "./writeCTAWPPhysSensitivityFiles <sub array> <observing time> <data directory> <1D/2D> <outputfile>" << endl;
	cout << endl;
	exit( 0 );
    }
    string fSubArray = argv[1];
    double fObservingTime_h = atof( argv[2] );
    string fDataDirectory = argv[3];
    string fHistogramDimension = argv[4];
    string fOutputFile = argv[5];

    VWPPhysSensitivityFile *iData = new VWPPhysSensitivityFile();
    iData->setSubArray( fSubArray );
    if( !iData->initializeOutputFile( fOutputFile ) ) exit( -1 );
    iData->setObservationTime( fObservingTime_h );
// binning according to file on the CTA MC Wiki and IFAE group
    iData->initializeHistograms( 20, -1.8, 2.2, 500, -1.8, 2.2, 400, -2.3, 2.7 );
    vector< double > iWobbleMin;
    vector< double > iWobbleMax;
    iData->initializeWobbleOffsets( iWobbleMin, iWobbleMax, fHistogramDimension );
// default Crab spectrum from HEGRA
    iData->setCrabSpectrum( "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat", 5 );
// CR spectra
    iData->setCosmicRaySpectrum( "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CR.dat", 0, 2 );

    if( !iData->fillHistograms( fDataDirectory ) )
    {
       exit( -1 );
    }

    iData->terminate();
}
