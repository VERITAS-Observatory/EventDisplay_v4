/*! \namespace VSkyCoordinatesUtilities
    \brief  utilities for timing and coordinate transformations

     Revision $Id$
*/

#include "VSkyCoordinatesUtilities.h"

double VSkyCoordinatesUtilities::getMJD( int i_year, int i_month, int i_day )
{
    double i_mjd = 0.;
    int i_stat = 0;
    slaCldj(i_year,i_month,i_day,&i_mjd,&i_stat);

    return i_mjd;
}


double VSkyCoordinatesUtilities::getUTC(int i_mjd, double i_seconds)
{
//! and add the fractional day to the mjd to get UTC
    double i_utc=i_mjd+i_seconds/24./60./60.;
    return i_utc;
}


void VSkyCoordinatesUtilities::rotate( const double theta_rad, double& x, double& y)
{
    const double s = sin(theta_rad);
    const double c = cos(theta_rad);
    const double _x = x*c - y*s;
    const double _y = y*c + x*s;
    x = _x;
    y = _y;
}


/*!
    calculate wobble offsets in ra,dec

    based on wobble.cpp from SFegan
*/
void VSkyCoordinatesUtilities::getWobbleOffsets( double iNorth, double iEast, double idec, double ira, double &idiffdec, double &idiffra )
{
    double degrad = 45./atan( 1. );

    idec /= degrad;
    ira /= degrad;

    double x = 0.;
    double y = 0.;
    double z = 1.;
    double theta_rad = sqrt(iNorth*iNorth + iEast*iEast ) / degrad;
    double phi_rad = -1.*atan2( iEast, iNorth );
    if( phi_rad < 0. ) phi_rad += TMath::TwoPi();

    VSkyCoordinatesUtilities::rotate( -theta_rad, z, x );
    VSkyCoordinatesUtilities::rotate( phi_rad, y, x );
// declination
    VSkyCoordinatesUtilities::rotate( TMath::PiOver2()-idec, z, x );
    idiffdec = (atan2( z, sqrt(x*x+y*y) ) - idec) * degrad;
// right ascension
    idiffra = atan2( y, x );
    if( idiffra < 0. ) idiffra += TMath::TwoPi();
    idiffra *= -1.*degrad;

    if( TMath::Abs( idiffra ) < 1.e-9 )   idiffra  = 0.;
    if( TMath::Abs( idiffdec ) < 1.e-9 )  idiffdec = 0.;
}


double VSkyCoordinatesUtilities::precessTarget( double iMJD, double &ra, double &dec )
{
    int  oy, om, od, j, ny, nd;
    double ofd,ofy;
    slaDjcl (iMJD, &oy, &om, &od, &ofd, &j);
    slaClyd (oy, om, od, &ny, &nd, &j);
    ofy=ny+nd/365.25;

    slaPreces("FK5",2000.0, ofy, &ra, &dec);

    return ofy;
}



/*!
    assume continuous increasing azimuth
*/
double VSkyCoordinatesUtilities::addToMeanAzimuth( double iMean, double iAz )
{
    if( iMean > 270. && iAz < 90. )
    {
        iMean += iAz + 360.;
    }
    else iMean += iAz;

    return iMean;
}

