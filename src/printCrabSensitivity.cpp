/* \file printCrabSensitivity
 * calculate sensitivity from Crab runs and
 * print in latex and markdown
 *
 *
*/

#include <iostream>
#include <string>
#include <vector>

#include "TFile.h"
#include "TTree.h"

#include "VSensitivityCalculator.h"

using namespace std;

void print_sensitivity( string anasum_file, double alpha = 1. / 6. )
{
	// get on and off counts
	TFile f( anasum_file.c_str() );
	if( f.IsZombie() )
	{
		cout << "Error reading anasum file " << anasum_file << endl;
		cout << "Exiting...";
		return;
	}
	TTree* t = ( TTree* )f.Get( "total_1/stereo/tRunSummary" );
	if( !t )
	{
		cout << "Error reading run summary tree from " << anasum_file << endl;
		cout << "Exiting...";
		return;
	}
	double Rate = 0;
	double RateOff = 0.;
	double RateE = 0;
	double RateOffE = 0.;
	t->SetBranchAddress( "Rate", &Rate );
	t->SetBranchAddress( "RateOff", &RateOff );
	t->SetBranchAddress( "RateE", &RateE );
	t->SetBranchAddress( "RateOffE", &RateOffE );

	t->GetEntry( t->GetEntries() - 1 );

	cout << "Rates (on/Off): " << Rate << "\t" << RateOff << endl;

	vector< double > fSourceStrength;
	fSourceStrength.push_back( 1. );
	fSourceStrength.push_back( 0.3 );
	fSourceStrength.push_back( 0.1 );
	fSourceStrength.push_back( 0.03 );
	fSourceStrength.push_back( 0.01 );
	fSourceStrength.push_back( 0.003 );
	VSensitivityCalculator a;
	a.setSourceStrengthVector_CU( fSourceStrength );
	a.addDataSet( Rate, RateOff, alpha, "" );
	a.list_sensitivity();
}

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		cout << endl;
		cout << "./printCrabSensitivity <anasum result file on Crab>" << endl;
		cout << endl;
		exit( EXIT_FAILURE );
	}
	print_sensitivity( argv[1] );
}
