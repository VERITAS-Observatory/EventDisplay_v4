/*! \file mscw_energy.cpp
    \brief calculate mean scaled width and length,  energy with lookup tables

    Revision $Id: mscw_energy.cpp,v 1.35.2.14.4.6.2.1.2.1.4.1.2.8.4.2.2.2.12.2 2010/09/01 12:20:23 gmaier Exp $

    \author
     Gernot Maier
     Henrik Krawczynski (mscw+energy routine)
*/

#include "VTableCalculator.h"
#include "VTableEnergyCalculator.h"
#include "VTableLookupDataHandler.h"
#include "VGlobalRunParameter.h"
#include "VTableLookupRunParameter.h"
#include "VTableLookup.h"

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
    VTableLookupRunParameter *fX = (VTableLookupRunParameter*)iF.Get( "TLRunParameter" );
    if( fX ) fX->print( 2 );
    else     cout << "no table lookup run parameters found" << endl;
    iF.Close();

    exit( 0 );
}


int main( int argc, char *argv[] )
{
// timing
    TStopwatch fStopWatch;
    fStopWatch.Start();

    VTableLookupRunParameter *fTLRunParameter = new VTableLookupRunParameter();
    fTLRunParameter->SetNameTitle( "TLRunParameter", fTLRunParameter->getEVNDISP_VERSION().c_str() );

    cout << endl;
    cout << "mscw_energy (" << fTLRunParameter->getEVNDISP_VERSION() << ")" << endl;
    cout << endl;
    cout << "calculation of mean scaled width and length, and energy with lookup tables" << endl;
    cout << "--------------------------------------------------------------------------" << endl;
    cout << endl;

    if( !fTLRunParameter->fillParameters( argc, argv ) ) exit( 0 );
    if( fTLRunParameter->printpara.size() > 0 )
    {
        printParametersFromFile( fTLRunParameter->printpara );
        exit( 0 );
    }
    fTLRunParameter->print();

// create lookup tables
    VTableLookup *fTLook = new VTableLookup( fTLRunParameter->readwrite, fTLRunParameter->fDebug );
    if( !fTLook->initialize( fTLRunParameter ) )
    {
       cout << "error creating lookup tables: no run parameters";
       cout << "exciting..." << endl;
       exit( -1 );
    }

    cout << endl << "loop over all events (in total " << fTLook->getNEntries() << ")" << endl;
    if( fTLook->getMaxTotalTime() < 1.e8 ) cout << "\t maximum run time [s]: " << fTLook->getMaxTotalTime() << endl;

    fTLook->loop();

    cout << "... end of loop" << endl;

    fStopWatch.Stop();
    fStopWatch.Print();

    fTLook->terminate();

}
