//! VSkyCoordinatesUtilities utilities for timing and coordinate transformations

#ifndef VSkyCoordinatesUtilities_H
#define VSkyCoordinatesUtilities_H

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "TF1.h"
#include "TMath.h"
#include "TSystem.h"
#include "TTree.h"

#include "VAstronometry.h"
#include "VGlobalRunParameter.h"

namespace VSkyCoordinatesUtilities
{
    double addToMeanAzimuth( double iMean, double iAz );                   // mean azimuth calculation
    double adjustAzimuthToRange( double );
    double angularDistance( double Az1_rad, double Ze1_rad, double Az2_rad, double Ze2_rad );
    void   convert_derotatedCoordinates_to_J2000( double iMJD, double i_RA_J2000_deg, double i_DEC_J2000_deg, double& x, double& y );
    void   getDifferenceInCameraCoordinates( double tel_ze, double tel_az, double shower_ze,  double shower_az, float& x, float& y, float& z );
    void   getEquatorialCoordinates( int MJD, double time, double az, double ze, double& dec, double& ra );
    double getDerotationAngle( double MJD, double time, double iTelRA, double iTelDec, double iObservatoryLongitude, double iObservatoryLatitude );
    double getDerotationAngle( double i_UTC, double iTelRA, double iTelDec, double iObservatoryLongitude, double iObservatoryLatitude );
    double getHourAngle( double i_UTC, double iTelRA, double iObservatoryLongitude );
    double getSidereal( double i_UTC, double iObservatoryLongitude );
    void   getHorizontalCoordinates( int MJD, double time, double dec_deg, double ra_deg, double& az_deg, double& ze_deg );
    double getMJD( int i_year, int i_month, int i_day );
    int    getMJD_from_SQLstring( string iSQLData, double& mjd, double& sec_of_day );
    string getSQLstring_fromMJD( double MJD );
    void   getRotatedShowerDirection( double ze, double az, double y, double x, double& rze, double& raz );
    double getTargetShiftWest( double iTargetRA_deg, double iTargetDec_deg, double ira_deg, double idec_deg );
    double getTargetShiftNorth( double iTargetRA_deg, double iTargetDec_deg, double ira_deg, double idec_deg );
    double getUTC( int i_mjd, double i_seconds );
    void   getWobbledDirection( double iNorth_deg, double iEast_deg, double idec_deg, double ira_deg, double& dec_W_deg, double& ra_W_deg );
    void   getWobbleOffset_in_RADec( double iNorth, double iEast, double idec, double ira, double& idiffdec, double& idiffra );
    void precessTarget( double iMJD_end, double& ra_rad, double& dec_rad, double iMJD_start = 51544., bool bUnitIsDegrees = false );     // default start is J2000
    void   rotate( const double theta_rad, double& x, double& y );
}

#endif
