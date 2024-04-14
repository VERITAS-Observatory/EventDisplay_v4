/*! \file printAnasumRunParameter
    \brief print anasum run parameters


*/

#include "VEvndispRunParameter.h"
#include "VTableLookupRunParameter.h"
#include "VGlobalRunParameter.h"
#include "VEvndispReconstructionParameter.h"
#include "VMonteCarloRunHeader.h"
#include "VAnaSumRunParameter.h"

#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <sstream>

using namespace std;

int main( int argc, char* argv[] )
{
	// print version only
	if( argc == 2 )
	{
		string fCommandLine = argv[1];
		if( fCommandLine == "-v" || fCommandLine == "--version" )
		{
			VGlobalRunParameter fRunPara;
			cout << fRunPara.getEVNDISP_VERSION() << endl;
			exit( 0 );
		}
	}

	if( argc < 3 )
	{
		cout << endl;
		cout << "printAnasumRunParameter " << VGlobalRunParameter::getEVNDISP_VERSION() << endl;
		cout << "==========================" << endl;
		cout << endl;
		cout << "usage: printAnasumRunParameter <file> <run number>" << endl;
		cout << endl;
		cout << "print anasum run parameters stored in anasum root file" << endl;
		cout << endl;
		cout << "   options: " << endl;
		cout << "      -effareafile  print effective area file" << endl;
		cout << "      -acceptancefile print radical acceptance file" << endl;
		cout << "      -cutfile      print cut file" << endl;
		cout << endl;
		exit( 0 );
	}
	// command line option
	int fRunNumber = 0;
	if( argc >= 3 )
	{
		fRunNumber = atoi( argv[2] );
	}
	// open file
	TFile* fIn = new TFile( argv[1] );
	if( fIn->IsZombie() )
	{
		cout << "error: file not found: " << argv[1] << endl;
		cout << "exiting..." << endl;
		exit( 0 );
	}
	ostringstream oss;
	oss << "run_" << fRunNumber << "/stereo/";
	if( fIn->cd( oss.str().c_str() ) )
	{
		if( argc == 4 )
		{
			string fOption = argv[3];
			VAnaSumRunParameter* a = ( VAnaSumRunParameter* )gDirectory->Get( "VAnaSumRunParameter" );
			if( a && a->fMapRunList.find( fRunNumber ) != a->fMapRunList.end() )
			{
				if( fOption == "-effareafile" )
				{
					cout << a->fMapRunList[fRunNumber].fEffectiveAreaFile << endl;
				}
				else if( fOption == "-acceptancefile" )
				{
					cout << a->fMapRunList[fRunNumber].fAcceptanceFile << endl;
				}
				else if( fOption == "-cutfile" )
				{
					cout << a->fMapRunList[fRunNumber].fCutFile << endl;
				}
				else if( fOption == "-teltoana" )
				{
					cout << a->fMapRunList[fRunNumber].fTelToAna << endl;
				}
				else if( fOption == "-ntel" )
				{
					cout << a->fMapRunList[fRunNumber].fNTel << endl;
				}
			}
		}
	}

	fIn->Close();

}
