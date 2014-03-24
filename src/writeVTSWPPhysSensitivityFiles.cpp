/* \file writeVTSWPPhysSensitivityFiles write VTS sensitivity files (CTA style)

   write a simple root file with histograms for sensitivities, effective areas,
   angular and energy resolution, migration matrix, etc.

   follow here CTA WP-Phys standards (with minor modifications)

   PRELIMINARY: many hardcoded values

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
	string fObservatory = "VTS";
	
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
}
