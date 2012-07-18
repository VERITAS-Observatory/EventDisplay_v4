/*! file VTS.getRunListFromDB read exposure from DB

    (VERITAS only)

    Revision $Id$

    \author Gareth Hughes
*/

#include "VGlobalRunParameter.h"
#include "VExposure.h"

#include <iostream>
#include <string>

int main( int argc, char *argv[] )
{
    cout << endl;
    cout << "VTS.getRunListFromDB (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
    cout << endl;
    if( argc != 8 )
    {
        cout << "./VTS.getRunListFromDB <db start time> <db stop time> <Source ID> <Minimum Elevation> <Minimum Run Duration [s]> <Find Laser Runs (1/0)> <Download Files (1/0)>" << endl;
        cout << endl;
        cout << "example: VTS.getRunListFromDB 2009-09-01 2009-10-01  Crab 60. 1200. 1 1" << endl;
        cout << "note: Using Laser Option extends run time." << endl;
	cout << endl;
        exit( 0 );
    }

    string iA = argv[1];
    string iB = argv[2];
    string iO = argv[3];
    double iE = atof(argv[4]);
    double iD = atof(argv[5]);
    int iL = atoi(argv[6]);
    int iN = atoi(argv[7]);

    VExposure a;
    a.setMakeRunList( true );
    a.setTimeRange( iA, iB );
    a.setSourceName( iO );
    a.setTelMinElevation( iE );
    a.setSelectLaser( iL );
    a.setMinDuration( iD );
    a.readFromDB();
    if( iL == 1 ) a.getLaserList();
    a.printListOfRuns();
    if( iN == 1 ) a.downloadRunList();
//    a.writeRootFile( iO );

}
