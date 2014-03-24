/*! \file run next day analysis and produce text and fits output


    \author Gernot Maier
*/

#include "VGlobalRunParameter.h"
#include "VAnalysisUtilities.h"
#include "VFITS.h"
#include "VRunList.h"
#include "VFluxCalculation.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void help()
{
	cout << endl;
	VGlobalRunParameter* iT = new VGlobalRunParameter();
	cout << "VERITAS_next_day (version " << iT->getEVNDISP_VERSION() << ")" << endl;
	cout << "===========================" << endl;
	cout << endl;
	cout << "VERITAS_next_day <anasum input file> <output file (.dat and .fits)> <target name> [merge information into one FITS-file=0/1 (default=1)] [debug=0/1 (default=0)]" << endl;
	cout << endl;
	exit( 0 );
}


int main( int argc, char* argv[] )
{
	bool fDebug = false;
	bool fMergeFITSFiles = true;
	
/////////////////////////////////////////
// read command line parameters
	if( argc == 1 )
	{
		help();
	}
	
	string fDataFile = argv[1];
	string fOUTFile = argv[2];
	string fTargetName = argv[3];
	if( argc >= 5 && atof( argv[4] ) == 0 )
	{
		fMergeFITSFiles = false;
	}
	if( argc == 6 && atof( argv[5] ) == 1 )
	{
		fDebug = true;
	}
	
/////////////////////////////////////////
// calculate total fluxes and upper limits
	double fMinEnergy = 0.2;
	double fGamma = 2.49;
	double iFlux , iFluxE, iFluxUL, iFluxInCU, iFluxULinCU , var1, var2;
	char ifile[100];
	sprintf( ifile, "%s", fDataFile.c_str() );
	//calculate fluxes for all runs even when significance < 3
	VFluxCalculation* flux = new VFluxCalculation( ifile );
	if( flux->IsZombie() )
	{
		return 1;
	}
	flux->setDebug( fDebug );
	flux->setSignificanceParameters( -5., -5. );
	flux->calculateFluxes( fMinEnergy, false );
	//flux->printResults();
//calculate upper limits for all runs even when significance >= 3
	VFluxCalculation* fluxUL = new VFluxCalculation( ifile );
	if( fluxUL->IsZombie() )
	{
		return 1;
	}
	fluxUL->setDebug( fDebug );
	fluxUL->setSignificanceParameters( 99999, 999999 );
	fluxUL->calculateFluxes( fMinEnergy, false );
	
	
// read run list
	VAnalysisUtilities a;
	a.openFile( fDataFile, -1, true, fDebug );
	if( !a.IsZombie() )
	{
		CRunSummary* c = a.getRunSummaryTree( -1 );
		
// open output stream
		ofstream fResults;
		fResults.open( ( fOUTFile + ".dat" ).c_str() );
		if( fResults && c )
		{
			for( int i = 0; i < c->fChain->GetEntries(); i++ )
			{
				c->GetEntry( i );
				if( i == 0 )
				{
				
					fResults << setw( 12 ) << left << "MJD" ;
					fResults << setw( 6 ) << left << "RA" ;
					fResults << setw( 6 ) << left << "Dec" ;
					fResults << setw( 7 ) << left << "runOn" ;
					fResults << setw( 5 ) << left << "NOn" ;
					fResults << setw( 5 ) << left << "NOff" ;
					fResults << setw( 18 ) << left << "tOn w/o deadtime";
					fResults << setw( 8 ) << left << "Signi" ;
					fResults << setw( 8 ) << left << "OffNorm" ;
					fResults << setw( 15 ) << left << "Rate" ;
					fResults << setw( 15 ) << left << "RateE" ;
					fResults << setw( 12 ) << left << "Flux" ;
					fResults << setw( 12 ) << left << "FluxE";
					fResults << setw( 14 ) << left << "FluxVsCrab";
					fResults << setw( 12 ) << left << "FluxUL";
					fResults << setw( 14 ) << left << "FluxULVsCrab";
					fResults << endl;
					fResults << setw( 12 ) << left << " " ;
					fResults << setw( 6 ) << left << "[deg]" ;
					fResults << setw( 6 ) << left << "[deg]" ;
					fResults << setw( 7 ) << left << " " ;
					fResults << setw( 5 ) << left << " " ;
					fResults << setw( 5 ) << left << " " ;
					fResults << setw( 18 ) << left << "[min]";
					fResults << setw( 8 ) << left << "[sigma]" ;
					fResults << setw( 8 ) << left << " " ;
					fResults << setw( 15 ) << left << "[gammas/min]" ;
					fResults << setw( 15 ) << left << "[gammas/min]" ;
					fResults << setw( 12 ) << left << "[m^-2 s^-1]" ;
					fResults << setw( 12 ) << left << "[m^-2 s^-1]";
					fResults << setw( 14 ) << left << "[percent]";
					fResults << setw( 12 ) << left << "[m^-2 s^-1]";
					fResults << setw( 14 ) << left << "[percent]";
					fResults << endl;
				}
				
				if( c->runOn > 0 )
				{
					flux->getFlux( c->runOn, iFlux, iFluxE, var1 );
					iFluxInCU = flux->getFluxVsCrab( iFlux, fMinEnergy , fGamma );
					fluxUL->getFlux( c->runOn, var1, var2, iFluxUL );
					iFluxULinCU = fluxUL->getFluxVsCrab( iFluxUL, fMinEnergy, fGamma );
					
					fResults.precision( 9 );
					fResults << setw( 12 ) << left << c->MJDOn;
					fResults.precision( 4 );
					fResults << setw( 6 ) << left << c->TargetRAJ2000 ;
					fResults << setw( 6 ) << left << c->TargetDecJ2000;
					fResults << setw( 7 ) << left << c->runOn ;
					fResults << setw( 5 ) << left << c->NOn ;
					fResults << setw( 5 ) << left << c->NOff ;
					fResults << setw( 18 ) << left << c->tOn / 60.*( 1. - c->DeadTimeFracOn ) ;
					fResults << setw( 8 ) << left << c->Signi;
					fResults << setw( 8 ) << left << c->OffNorm ;
					fResults << setw( 15 ) << left << c->Rate ;
					fResults << setw( 15 ) << left << c->RateE ;
					fResults << setw( 12 ) << left << iFlux ;
					fResults << setw( 12 ) << left << iFluxE;
					fResults << setw( 14 ) << left << iFluxInCU;
					fResults << setw( 12 ) << left << iFluxUL ;
					fResults << setw( 14 ) << left << iFluxULinCU ;
					fResults << endl;
				}
				else
				{
					flux->getFlux( -1, iFlux, iFluxE, var1 );
					iFluxInCU = flux->getFluxVsCrab( iFlux, fMinEnergy, fGamma );
					fluxUL->getFlux( -1, var1, var2, iFluxUL );
					iFluxULinCU = fluxUL->getFluxVsCrab( iFluxUL, fMinEnergy, fGamma );
					
					fResults << setw( 12 ) << left << "Total:";
					fResults << setw( 6 ) << left << c->TargetRAJ2000 ;
					fResults << setw( 6 ) << left << c->TargetDecJ2000 ;
					fResults << setw( 7 ) << left << 0.0 ;
					fResults << setw( 5 ) << left << c->NOn ;
					fResults << setw( 5 ) << left << c->NOff ;
					fResults << setw( 18 ) << left << c->tOn / 60.*( 1. - c->DeadTimeFracOn )  ;
					fResults << setw( 8 ) << left << c->Signi ;
					fResults << setw( 8 ) << left << 0.0 ;
					fResults << setw( 15 ) << left << c->Rate ;
					fResults << setw( 15 ) << left << c->RateE ;
					fResults << setw( 12 ) << left << iFlux ;
					fResults << setw( 12 ) << left << iFluxE ;
					fResults << setw( 14 ) << left << iFluxInCU;
					fResults << setw( 12 ) << left << iFluxUL ;
					fResults << setw( 14 ) << left << iFluxULinCU ;
					fResults << endl;
				}
				
			}
		}
		
/////////////////////////////////////////
// convert to fits

		cout << "---------Start production of FITS ouput file ------" << endl;
		VFITS f( fDataFile, fOUTFile + ".fits" , fTargetName, fMergeFITSFiles, fDebug );
		f.writeCumSignificance( fDebug );
		f.writeSignificanceDistribution( fDebug );
		f.writeLightCurve( fDebug );
		f.writeThetaSquareDistribution( fDebug );
		f.writeEnergySpectrum( fDebug );
		f.writeSignificanceSkyMap( fDebug );
		f.writeExcessSkyMap( fDebug );
		f.writeFITSFile( fDebug );
		
	}
}
