//! VSkyCoordinatesUtilities utilities for timing and coordinate transformations
// Revision $Id$

#ifndef VSkyCoordinatesUtilities_H
#define VSkyCoordinatesUtilities_H

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "TF1.h"
#include "TMath.h"
#include "TSystem.h"
#include "TTree.h"

#include "VASlalib.h"
#include "VGlobalRunParameter.h"

namespace VSkyCoordinatesUtilities
{
   double addToMeanAzimuth( double iMean, double iAz );                   // mean azimuth calculation
   double adjustAzimuthToRange( double );
   double angularDistance( double Az1_rad, double Ze1_rad, double Az2_rad, double Ze1_rad );
   void   convert_derotatedCoordinates_to_J2000( double iMJD, double i_RA_J2000_deg, double i_DEC_J2000_deg, double &x, double &y );
   void   getDifferenceInCameraCoordinates( double tel_ze, double tel_az, double shower_ze,  double shower_az, float &x, float &y, float &z );
   double getMJD( int i_year, int i_month, int i_day );
   void   getRotatedShowerDirection( double ze, double az, double y, double x, double &rze, double &raz );
   double getTargetShiftWest( double iTargetRA_deg, double iTargetDec_deg, double ira_deg, double idec_deg );
   double getTargetShiftNorth( double iTargetRA_deg, double iTargetDec_deg, double ira_deg, double idec_deg );
   double getUTC(int i_mjd, double i_seconds);
   void   getWobbleOffset_in_RADec( double iNorth, double iEast, double idec, double ira, double &idiffdec, double &idiffra );
   double precessTarget( double iMJD_end, double &ra_rad, double &dec_rad, double iMJD_start = 51544., bool bUnitIsDegrees = false );     // default start is J2000
   void   rotate( const double theta_rad, double& x, double& y);
}

#endif

