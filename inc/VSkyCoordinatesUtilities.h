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
   double getMJD( int i_year, int i_month, int i_day );
   double getUTC(int i_mjd, double i_seconds);
   void   getWobbleOffsets( double iNorth, double iEast, double idec, double ira, double &idiffdec, double &idiffra );
   double precessTarget( double iMJD, double &ra, double &dec );
   void   rotate( const double theta_rad, double& x, double& y);
}

#endif

