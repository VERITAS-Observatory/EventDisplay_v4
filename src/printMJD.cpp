/*! \file printMJD
 *
 *  print MJD from SQL string
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
        cout << "printMJD <SQL date>" << endl;
        cout << endl;
        cout << "print MJD from sql string" << endl;
        exit( EXIT_SUCCESS );
    }
    string iSQLData = argv[1];

    double mjd = 0.;
    double sec_of_day = 0;
    VSkyCoordinatesUtilities::getMJD_from_SQLstring(iSQLData, mjd, sec_of_day );

    cout << setprecision( 18 ) << mjd + sec_of_day/86400. << endl;
}
