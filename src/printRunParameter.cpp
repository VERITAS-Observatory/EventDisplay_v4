/*! \file printRunParameter
    \brief print run parameters from mscw or eventdisplay file to screen


    \author Gernot Maier
*/

#include "VEvndispRunParameter.h"
#include "VTableLookupRunParameter.h"
#include "VGlobalRunParameter.h"
#include "VEvndispReconstructionParameter.h"
#include "VMonteCarloRunHeader.h"

#include "TFile.h"
#include "TTree.h"

#include <iostream>

using namespace std;

bool readRunParameter( TFile* fIn, string iPara )
{
	if( !fIn )
	{
		return false;
	}
	
	VEvndispRunParameter* fPar = 0;
	
	fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterV2" );
	if( !fPar )
	{
		fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterDST" );
	}
	if( !fPar )
	{
		return false;
	}
	
	if( iPara == "-mcsourcefile" )
	{
		cout << fPar->fsourcefile << endl;
	}
        else if( iPara == "-date" )
        { 
                cout << fPar->fDBRunStartTimeSQL << endl;
        }
        else if( iPara == "-teltoana" )
        {
               for( unsigned int i = 0; i < fPar->fTelToAnalyze.size(); i++ )
               {
                   cout << fPar->fTelToAnalyze[i]+1;
               }
               cout << endl;
        }
	
	return true;
}


bool readMCParameter( TFile* fIn, string iPara )
{
	if( !fIn )
	{
		return false;
	}
	
	VMonteCarloRunHeader* fMC = 0;
	
	fMC = ( VMonteCarloRunHeader* )fIn->Get( "MC_runheader" );
	if( !fMC )
	{
		return false;
	}
	
	if( iPara == "-mcaz" )
	{
		fMC->printMCAz();
	}
	else if( iPara == "-runnumber" )
	{
		fMC->printRunNumber();
	}
	else
	{
		return false;
	}
	
	return true;
}

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

	if( argc != 2 && argc != 3 )
	{
		cout << endl;
		cout << "printRunParameter " << VGlobalRunParameter::getEVNDISP_VERSION() << endl;
		cout << "==========================" << endl;
		cout << endl;
		cout << "usage: printRunParameter <file> [opt]" << endl;
		cout << endl;
		cout << "print run parameters stored in eventdisplay or mscw_energy file" << endl;
		cout << endl;
		cout << "   options: " << endl;
		cout << "      -mcaz         print MC azimuth angle" << endl;
		cout << "      -runnumber    print MC run number" << endl;
		cout << "      -mcsourcefile print source file name" << endl;
                cout << "      -date         print date of run" << endl;
                cout << "      -teltoana     print telescope combination used in analysis" << endl;
		cout << endl;
		exit( 0 );
	}
	// command line option
	string fOption = "";
	if( argc == 3 )
	{
		fOption = argv[2];
	}
	if( fOption.size() == 0 )
	{
		cout << endl;
		cout << "printRunParameter " << VGlobalRunParameter::getEVNDISP_VERSION() << endl;
		cout << "==========================" << endl;
	}
	
	// open file
	TFile* fIn = new TFile( argv[1] );
	if( fIn->IsZombie() )
	{
		cout << "error: file not found: " << argv[1] << endl;
		cout << "exiting..." << endl;
		exit( 0 );
	}
	
	if( fOption.size() > 0 )
	{
		if( fOption == "-mcaz" || fOption == "-runnumber" )
		{
			readMCParameter( fIn, fOption );
		}
		else
		{
			readRunParameter( fIn, fOption );
		}
		exit( 0 );
	}
	
	VEvndispRunParameter* fPar = 0;
	
	fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterV2" );
	if( !fPar )
	{
		fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterDST" );
	}
	
	if( fPar )
	{
		if( fPar->fEventDisplayUser != "CTA-DST" )
		{
			fPar->print( 2 );
		}
		else
		{
			fPar->printCTA_DST();
		}
	}
	
	// array analysis cuts
	
	VEvndispReconstructionParameter* fArrayCuts = 0;
	
	fArrayCuts = ( VEvndispReconstructionParameter* )fIn->Get( "EvndispReconstructionParameter" );
	if( fArrayCuts )
	{
		cout << endl << endl;
		cout << "===========================================" << endl;
		cout << "===========================================" << endl;
		fArrayCuts->print_arrayAnalysisCuts();
	}
	
	VTableLookupRunParameter* fTPar = 0;
	
	fTPar = ( VTableLookupRunParameter* )fIn->Get( "TLRunParameter" );
	
	if( fTPar )
	{
		cout << endl << endl;
		cout << "===========================================" << endl;
		cout << "===========================================" << endl;
		fTPar->print( 2 );
	}
	
	VMonteCarloRunHeader* fMC = 0;
	
	//    if( fPar && fPar->fEventDisplayUser != "CTA-DST" )
	{
		fMC = ( VMonteCarloRunHeader* )fIn->Get( "MC_runheader" );
		if( fMC )
		{
			cout << endl << endl;
			cout << "===========================================" << endl;
			cout << "===========================================" << endl;
			fMC->print();
		}
	}
	
	
	fIn->Close();
	
}
