//! VSkyCoordinates get pointing direction of telescope
// Revision $Id: VSkyCoordinates.h,v 1.10.8.6.12.2.6.1.2.3.10.1.2.1 2010/11/09 14:38:06 gmaier Exp $

#ifndef VSKYCOORDINATES_H
#define VSKYCOORDINATES_H

#include "TMath.h"

#include <iomanip>
#include <iostream>

#include "VASlalib.h"
#include "VSkyCoordinatesUtilities.h"
#include "VStarCatalogue.h"
#include "VTargets.h"

using namespace std;

class VSkyCoordinates
{
    protected:

	VStarCatalogue *fStarCatalogue;

        bool   fSet;                              //!< true if target or dec/ra is set
        bool   fPrecessed;                        //!< true if target position has been precessed
        bool   fMC;                               //!< true for Monte Carlo run

        unsigned int fMJD;
        double fTime;
        double fTelDec;                           //!< [rad] //! declination of pointing direction
        double fTelRA;                            //!< [rad] //! right ascension of pointing direction
        float  fTelDec_deg;                       //!< [deg] //! declination of pointing direction
        float  fTelRA_deg;                        //!< [deg] //! right ascension of pointing direction
        double fTargetDec;                        //!< [rad] //! declination of target
        double fTargetRA;                         //!< [rad] //! right ascension of target
        string fTargetName;
        double fTargetElevation;                  //!< [deg]
        double fTargetAzimuth;                    //!< [deg]
        bool   fWobbleSet;                        //!< be sure that wobble offset is only applied once
        double fWobbleNorth;                      //!< [deg] wobble offset north
        double fWobbleEast;                       //!< [deg] wobble offset east

// telescope orientation
        double fTelAzimuth;                       //!< [deg]  return value to be used in the analysis
        double fTelElevation;                     //!< [deg]  return value to be used in the analysis
	float  fTelAzimuthCalculated;             //!< [deg]  elevation from source coordinates
	float  fTelElevationCalculated;           //!< [deg]  elevation from source coordinates

        double fObsLatitude;                      //!< [rad]
        double fObsLongitude;                     //!< [rad]

        void reset();

    public:

        VSkyCoordinates();
       ~VSkyCoordinates() {}

        double adjustAzimuthToRange( double );
        void   derotateCoords( double i_UTC, double i_xin, double i_yin, double & i_xout, double & i_yout);
        double getDerotationAngle(double i_UTC);
        void   getEquatorialCoordinates( int MJD, double time, double az, double ze, double &dec, double &ra );
        double getHourAngle(double i_UTC);
        double getSidereal(double i_UTC);
        string getTargetName() { return fTargetName; }
        void   getRotatedShowerDirection( double y, double x, double &rze, double &raz );
        void   getDerotatedShowerDirection( double ze, double az, float &y, float &x, double rze, double raz );
        double getTargetDec() { return fTargetDec * 180./TMath::Pi(); }
        double getTargetRA() { return fTargetRA * 180./TMath::Pi(); }
        double getTargetElevation() { return fTargetElevation; }
        double getTargetAzimuth()   { return fTargetAzimuth; }
        double getTelAzimuth()      { return fTelAzimuth; }
        double getTelElevation()    { return fTelElevation; }
        double getTelDec() { return fTelDec * 180./TMath::Pi(); }
        double getTelRA() { return fTelRA * 180./TMath::Pi(); }
        double getTelLatitude() { return fObsLatitude*45./atan(1.); }
        double getTelLongitude() { return fObsLongitude*45./atan(1.); }
	VStarCatalogue* getStarCatalogue() { return fStarCatalogue; }
        double getWobbleNorth() { return fWobbleNorth; }
        double getWobbleEast() { return fWobbleEast; }
	bool   initStarCatalogue( string iCatalogueName, double iMJD, double xmin, double xmax, double ymin, double ymax,
	                                                 double iRASkyMapCentreJ2000, double iDecSkyMapCentreJ2000 );
        bool   isPrecessed() { return fPrecessed; }
        bool   isSet() { return fSet; }
        void   precessTarget ( int iMJD, int iTelID = -1 );
        void   rotateCoords( int i_mjd, double i_seconds, double i_xin, double i_yin, double & i_xout, double & i_yout);
        void   setMC() { fMC = true; }
	void   setObservatory( double iLongitude_deg = 0., double iLatitude_deg = 0. );
        bool   setPointingOffset( double i_raOff, double i_decOff );
        bool   setTarget( string iTargetName );
        bool   setTarget( double iDec, double iRA );
        void   setTargetName( string iTargetName ) { fTargetName = iTargetName; }
	void   setTelDec_deg( double iTelDec_deg ) { fTelDec = iTelDec_deg * TMath::DegToRad(); }
	void   setTelRA_deg( double iTelRA_deg )   { fTelRA  = iTelRA_deg  * TMath::DegToRad(); }
        void   setTelAzimuth( double iTelAz )      { fTelAzimuth = iTelAz; }           //!< set telescope azimuth (e.g.for MC)
        void   setTelElevation( double iTelEl )    { fTelElevation = iTelEl; }         //!< set telescope elevation (e.g. for MC)
        void   setWobbleOffset( double iWobbleNorth, double iWobbleEast, unsigned int iTelID );
        void   updatePointing( int MJD, double time );
};
#endif
