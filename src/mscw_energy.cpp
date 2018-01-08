/*! \file mscw_energy.cpp
    \brief calculate mean scaled width and length,  energy with lookup tables

    \author
     Gernot Maier
     basic routines: Henrik Krawczynski (mscw+energy routine)
*/

#include "VTableCalculator.h"
#include "VTableEnergyCalculator.h"
#include "VTableLookupDataHandler.h"
#include "VGlobalRunParameter.h"
#include "VTableLookupRunParameter.h"
#include "VTableLookup.h"

#include <TChain.h>
#include <TStopwatch.h>

#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

void printParametersFromFile( string ff )
{
	TFile iF( ff.c_str() );
	if( iF.IsZombie() )
	{
		cout << "couldn't read mscw file: " << ff << endl;
		exit( 0 );
	}
	VTableLookupRunParameter* fX = ( VTableLookupRunParameter* )iF.Get( "TLRunParameter" );
	if( fX )
	{
		fX->print( 2 );
	}
	else
	{
		cout << "no table lookup run parameters found" << endl;
	}
	iF.Close();
	
	exit( 0 );
}

///////////////////////////////////////////////////////////////////////////////

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
	// timing
	TStopwatch fStopWatch;
	fStopWatch.Start();
	
	VTableLookupRunParameter* fTLRunParameter = new VTableLookupRunParameter();
	fTLRunParameter->SetNameTitle( "TLRunParameter", fTLRunParameter->getEVNDISP_VERSION().c_str() );
	
	cout << endl;
	cout << "mscw_energy (" << fTLRunParameter->getEVNDISP_VERSION() << ")" << endl;
	cout << "=======================" << endl;
	cout << endl;
	cout << "calculation of mean scaled width and length, and energy with lookup tables" << endl;
	cout << "--------------------------------------------------------------------------" << endl;
	cout << endl;
	
	if( !fTLRunParameter->fillParameters( argc, argv ) )
	{
		exit( 0 );
	}
	
	if( fTLRunParameter->printpara.size() > 0 )
	{
		printParametersFromFile( fTLRunParameter->printpara );
		exit( 0 );
	}
	fTLRunParameter->print();
	
	// create lookup tables
	VTableLookup* fTLook = new VTableLookup( fTLRunParameter->readwrite, fTLRunParameter->fDebug );
	if( !fTLook->initialize( fTLRunParameter ) )
	{
		cout << "error creating lookup tables: no run parameters";
		cout << "exiting..." << endl;
		exit( -1 );
	}
	
	cout << endl << "loop over all events ";
        cout << "(in total " << fTLook->getNEntries() << ")";
	cout << endl;
	if( fTLook->getMaxTotalTime() < 1.e8 )
	{
		cout << "\t maximum run time [s]: " << fTLook->getMaxTotalTime() << endl;
	}
	
	// loop over all events
	fTLook->loop();
	
	cout << "... end of loop" << endl;
	
	fStopWatch.Stop();
	fStopWatch.Print();
	
	// write tables to disk
	fTLook->terminate();
}
