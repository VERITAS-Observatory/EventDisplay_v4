//! VAstroSource   class defining an astronomical source
//  Revision $Id: VAstroSource.h,v 1.3.8.2.20.1.10.1 2010/03/08 07:45:08 gmaier Exp $

#ifndef VASTROSOURCE_H
#define VASTROSOURCE_H

#include "VStarCatalogue.h"

#include "TMath.h"

#include <iostream>
#include <string>

#include "VASlalib.h"

using namespace std;

class VAstroSource;

class VAstroSource
{
    public:
        VAstroSource( double i_RA_trk, double i_dec_trk, double i_RA_src, double i_dec_src );
        ~VAstroSource();
        void init( string, double, double, double, double, double, double irc = 0., double idc = 0. );
	void setObservatory( double iLongitude, double iLatitude );
        void setTrackedRAdec(double i_RA,double i_dec);
        void setSourceRAdec(double i_RA,double i_dec);

        void rotateCoords( double i_UTC, double i_xin, double i_yin, double & i_xout, double & i_yout);
        void derotateCoords( double i_UTC, double  i_xin, double i_yin, double & i_xout, double &i_yout);
        double getSidereal(double i_UTC);
        double getHourAngle(double i_UTC);
        double getDerotationAngle(double i_UTC);
        void calcCameraCoords(double & i_x, double & i_y);
        double getSourceRA(){ return fRA_src; }
        double getSourceDec(){ return fDec_src; }
        double getTrackedRA(){ return fRA_trk; }
        double getTrackedDec(){ return fDec_trk; }
        double getSourceX(){ return fX_src; }
        double getSourceY(){ return fY_src; }
        VStarCatalogue* getStarCatalogue() { return fStarCatalogue; }

    private:

        double fLongitude;                 //!< Telescope longitude (radians)
        double fLatitude;                  //!< Telescope latitude (radians)
        double fRA_trk;                           //!< Right ascension of tracked point (radians)
        double fDec_trk;                          //!< Declination of tracked point (radians)
        double fRA_src;                           //!< Right ascension of source (radians)
        double fDec_src;                          //!< Declination of source (radians)
        double fX_src;                            //!< X position of source in the camera (degrees)
        double fY_src;                            //!< Y position of source in the camera (degrees)

        VStarCatalogue *fStarCatalogue;
};
#endif
