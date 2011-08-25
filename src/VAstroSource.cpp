/*! \class VAstroSource
    \brief class for describing an astronomical source
    \author
      Jamie Holder 
      Gernot Maier

      Revision $Id: VAstroSource.cpp,v 1.4.8.2.20.2.10.1 2010/03/08 07:45:08 gmaier Exp $
*/

#include "VAstroSource.h"
/*!
 *   all angles in [rad]
 *
 */
VAstroSource::VAstroSource(double i_RA_trk, double i_dec_trk, double i_RA_src, double i_dec_src )
{
// tracking position
    fRA_trk=i_RA_trk;
    fDec_trk=i_dec_trk;

// source position
    fRA_src=i_RA_src;
    fDec_src=i_dec_src;

// observatory
    fLatitude = 0.;
    fLongitude = 0.;

    fStarCatalogue = new VStarCatalogue();
}


VAstroSource::~VAstroSource()
{
}


void VAstroSource::init( string iCatalogue, double iMJD, double xmin, double xmax, double ymin, double ymax, double iRASkyMapCentre, double iDecSkyMapCentre )
{
    if( fStarCatalogue )
    {
        double i_x = 0.;
        double i_y = 0.;
        if( fabs( xmin ) > fabs( xmax ) ) i_x = fabs( xmin );
        else                              i_x = fabs( xmax );
        if( fabs( ymin ) > fabs( ymax ) ) i_y = fabs( ymin );
        else                              i_y = fabs( ymax );
        if( !fStarCatalogue->init( iMJD, iCatalogue ) )
        {
            cout << "Error reading star catalogue: " << iCatalogue << endl;
            cout << "exiting..." << endl;
            exit( 0 );
        }
        fStarCatalogue->setFOV( iRASkyMapCentre, iDecSkyMapCentre, i_x, i_y, true );
    }
}


void VAstroSource::setTrackedRAdec(double i_RA, double i_dec)
{
    fRA_trk=i_RA;
    fDec_trk=i_dec;
}


void VAstroSource::setSourceRAdec(double i_RA, double i_dec)
{
    fRA_src=i_RA;
    fDec_src=i_dec;
}


double VAstroSource::getSidereal(double i_UTC)
{
    return slaDranrm(slaGmst(i_UTC)-fLongitude);
}


double VAstroSource::getHourAngle(double i_UTC)
{
    return slaDranrm(getSidereal(i_UTC)-fRA_trk);
}


double VAstroSource::getDerotationAngle(double i_UTC)
{
//! Returns the camera derotation angle in radians
    return -slaPa(getHourAngle(i_UTC),fDec_trk,fLatitude);
}


void VAstroSource::derotateCoords( double i_UTC, double i_xin, double i_yin, double & i_xout, double & i_yout)
{
    double i_theta=getDerotationAngle(i_UTC);
    i_xout=i_xin*cos(i_theta)+i_yin*sin(i_theta);
    i_yout=i_yin*cos(i_theta)-i_xin*sin(i_theta);
}


void VAstroSource::rotateCoords( double i_UTC, double i_xin, double i_yin, double & i_xout, double & i_yout)
{
    double i_theta = -1. * getDerotationAngle(i_UTC);
    i_xout=i_xin*cos(i_theta)+i_yin*sin(i_theta);
    i_yout=i_yin*cos(i_theta)-i_xin*sin(i_theta);
}


void VAstroSource::calcCameraCoords(double & i_x, double & i_y)
{

    int i_err;

//! calculates the position on the tangent plane.
    slaDs2tp (fRA_src, fDec_src, fRA_trk,fDec_trk,&fX_src,&fY_src, &i_err);

//! camera position is more useful in degrees
    fX_src=fX_src*180./M_PI;
    fY_src=fY_src*180./M_PI;

//! invert the y direction. Need this for Whipple - not sure about VERITAS?
    fY_src*=-1.0;

    i_x=fX_src;
    i_y=fY_src;

}

void VAstroSource::setObservatory( double iLongitude, double iLatitude )
{
   fLatitude = iLatitude / 180.0*M_PI;
   fLongitude = iLongitude / 180.0*M_PI;
}
