/*! \file printSQLDate
 *
 *  print SQL date string from MJD
 *
 */

#include <iomanip>
#include <iostream>

#include "VSkyCoordinatesUtilities.h"

using namespace std;

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		cout << "printMJD <MJD>" << endl;
		cout << endl;
		cout << "print SQL-formated date string from MJD (ignoring hours and seconds)" << endl;
		exit( EXIT_SUCCESS );
	}
	double iMJD = atof(argv[1]);
	
	cout << VSkyCoordinatesUtilities::getSQLstring_fromMJD(iMJD) << endl;
}
