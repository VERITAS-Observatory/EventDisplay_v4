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
    idec /= TMath::RadToDeg();
    ira  /= TMath::RadToDeg();

    double x = 0.;
    double y = 0.;
    double z = 1.;
    double theta_rad = sqrt(iNorth*iNorth + iEast*iEast ) / TMath::RadToDeg();
    double phi_rad = -1.*atan2( iEast, iNorth );
    if( phi_rad < 0. ) phi_rad += TMath::TwoPi();

    VSkyCoordinatesUtilities::rotate( -theta_rad, z, x );
    VSkyCoordinatesUtilities::rotate( phi_rad, y, x );
// declination
    VSkyCoordinatesUtilities::rotate( TMath::PiOver2()-idec, z, x );
    idiffdec = (atan2( z, sqrt(x*x+y*y) ) - idec) * TMath::RadToDeg();
// right ascension
    idiffra = atan2( y, x );
    if( idiffra < 0. ) idiffra += TMath::TwoPi();
    idiffra *= -1.*TMath::RadToDeg();

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


/*
     difference between to pointing directions in camera coordinates

     input coordinates in [deg]
*/
void VSkyCoordinatesUtilities::getDifferenceInCameraCoordinates( double tel_ze, double tel_az, 
                                                        double shower_ze,  double shower_az, 
							float &x, float &y, float &z )
{
// convert coordinates from [deg] to [rad]
    tel_az    /= TMath::RadToDeg();
    shower_az /= TMath::RadToDeg();
    double tel_el    = (90.-tel_ze)/TMath::RadToDeg();
    double shower_el = (90.-shower_ze)/TMath::RadToDeg();

    double cx = cos( shower_el ) * sin( shower_az );
    double cy = cos( shower_el ) * cos( shower_az );
    double cz = sin( shower_el );

    double i_temp = sin( tel_az ) * cx + cos( tel_az ) * cy;

    x = (cos( tel_az ) * cx - sin( tel_az ) * cy) * TMath::RadToDeg();
    z = (cos( tel_el ) * i_temp + sin( tel_el ) * cz);
    y = (-1.*sin( tel_el ) * i_temp + cos( tel_el ) * cz) * TMath::RadToDeg();
    y *= -1.;

    if( fabs( x ) < 1.e-4 ) x = 0.;
    if( fabs( y ) < 1.e-4 ) y = 0.;
    if( fabs( z ) < 1.e-4 ) z = 0.;
}

/*
    calculate shower direction from telescope pointing and reconstruction shower direction

     small angle approximation, assume small x,y (neglect z)
*/

void VSkyCoordinatesUtilities::getRotatedShowerDirection( double ze, double az, double y, double x, double &rze, double &raz )
{
// get all directions in [rad]
    x /= TMath::RadToDeg();
    y /= (-1.*TMath::RadToDeg());
// assume all telescopes point in same directions
    double el = (90.-ze)/TMath::RadToDeg();
    az = az/TMath::RadToDeg();
// these are the resulting directions

    double r = sqrt( 1. + x*x + y*y );
    double cx = x/r;
    double cy = 1./r;
    double cz = y/r;

// rotate telescope around elevation axis
    double ex = cx;
    double ey = cy * cos( el ) - cz * sin( el );
    double ez = cy * sin( el ) + cz * cos( el );
// rotate around azimuth
    double rx, ry, rz;
    rx =     ex * cos( az ) + ey * sin( az );
    ry = -1.*ex * sin( az ) + ey * cos( az );
    rz = ez;
// calculate new azimuth, zenith
    r = sqrt( rx*rx + ry*ry );
// small value check
    if( fabs(r) < 1.e-10 ) r = 0.;
    if( fabs(rx) < 1.e-10 ) rx = 0.;
    if( fabs(ry) < 1.e-10 ) ry = 0.;
    if( fabs(rz) < 1.e-10 ) rz = 0.;

    if( r == 0. ) raz = az * TMath::RadToDeg();
    else
    {
        raz = (TMath::Pi()/2.-atan2(ry,rx))*TMath::RadToDeg();
        if( raz > 180. )  raz = -1.*(360.-raz);
        if( raz < -180. ) raz *= -1.;
    }
    if( rz == 0. ) rze = 90. - el*TMath::RadToDeg();
    else
    {
        rze = 90.-atan2(rz,r)*TMath::RadToDeg();
    }
}


double VSkyCoordinatesUtilities::adjustAzimuthToRange( double az )
{
    return slaDranrm( az/TMath::RadToDeg()) * TMath::RadToDeg();
}

