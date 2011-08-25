/*! \file printRunParameter
    \brief print run parameters from mscw or eventdisplay file to screen

    Revision $Id: printRunParameter.cpp,v 1.1.2.2.4.1.12.2.14.1.4.2.2.1.2.3 2011/02/14 12:45:28 gmaier Exp $

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

int main( int argc, char *argv[] )
{
    cout << endl;
    cout << "printRunParameter " << VGlobalRunParameter::getEVNDISP_VERSION() << endl;
    cout << "==========================" << endl;
    if( argc != 2 )
    {
        cout << endl;
        cout << "usage: printRunParameter <file>" << endl;
        cout << endl;
        cout << "print run parameters stored in eventdisplay or mscw_energy file" << endl;
        cout << endl;
        exit( 0 );
    }

    TFile *fIn = new TFile( argv[1] );
    if( fIn->IsZombie() )
    {
        cout << "error: file not found: " << argv[1] << endl;
        cout << "exiting..." << endl;
        exit( 0 );
    }

    VEvndispRunParameter *fPar = 0;

    fPar = (VEvndispRunParameter*)fIn->Get( "runparameterV2" );
    if( !fPar ) fPar = (VEvndispRunParameter*)fIn->Get( "runparameterDST" );

    if( fPar )
    {
        if( fPar->fEventDisplayUser != "CTA-DST" ) fPar->print( 2 );
	else                                       fPar->printCTA_DST();
    }

// array analysis cuts
  
    VEvndispReconstructionParameter *fArrayCuts = 0;

    fArrayCuts = (VEvndispReconstructionParameter*)fIn->Get( "EvndispReconstructionParameter" );
    if( fArrayCuts )
    {
        cout << endl << endl;
        cout << "===========================================" << endl;
        cout << "===========================================" << endl;
	fArrayCuts->print_arrayAnalysisCuts();
    }

    VTableLookupRunParameter *fTPar = 0;

    fTPar = (VTableLookupRunParameter*)fIn->Get( "TLRunParameter" );

    if( fTPar )
    {
        cout << endl << endl;
        cout << "===========================================" << endl;
        cout << "===========================================" << endl;
        fTPar->print( 2 );
    }

    VMonteCarloRunHeader *fMC = 0;

//    if( fPar && fPar->fEventDisplayUser != "CTA-DST" )
    {
       fMC = (VMonteCarloRunHeader*)fIn->Get( "MC_runheader" );
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
