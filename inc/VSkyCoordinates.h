//! VSkyCoordinates get pointing direction of telescope
// Revision $Id: VSkyCoordinates.h,v 1.10.8.6.12.2.6.1.2.3.10.1.2.1 2010/11/09 14:38:06 gmaier Exp $

#ifndef VSKYCOORDINATES_H
#define VSKYCOORDINATES_H

#include "TMath.h"
#include "TTree.h"

#include <iomanip>
#include <iostream>

#include "VASlalib.h"
#include "VPointingDB.h"
#include "VSkyCoordinatesUtilities.h"
#include "VTargets.h"

using namespace std;

class VSkyCoordinates
{
    private:
        unsigned int fTelID;

        VTargets *fTarget;

        bool   fSet;                              //!< true if target or dec/ra is set
        bool   fPrecessed;                        //!< true if target position has been precessed
        bool   fMC;                               //!< true for Monte Carlo run

        unsigned int fMJD;
        double fTime;
        double fTelDec;                           //!< [rad] //! declination of pointing direction
        double fTelRA;                            //!< [rad] //! right ascension of pointing direction
        float fTelDec_deg;                        //!< [deg] //! declination of pointing direction
        float fTelRA_deg;                         //!< [deg] //! right ascension of pointing direction
        double fTargetDec;                        //!< [rad] //! declination of target
        double fTargetRA;                         //!< [rad] //! right ascension of target
        string fTargetName;
        double fTargetElevation;                  //!< [deg]
        double fTargetAzimuth;                    //!< [deg]
        bool   fWobbleSet;                        //!< be sure that wobble offset is only applied once
        double fWobbleNorth;                      //!< [deg] wobble offset north
        double fWobbleEast;                       //!< [deg] wobble offset east

        double fTelAzimuth;                       //!< [deg]
        double fTelElevation;                     //!< [deg]
        float fTelAzimuth_float;                  //!< [deg]
        float fTelElevation_float;                //!< [deg]

        double fObsLatitude;                      //!< [rad]
        double fObsLongitude;                     //!< [rad]

        int bPointingError;                       //!< 0: no pointing error, 1: from command line, 2: from tracking code (DB), 3: from pointing monitor text file, 4: from pointing monitor (DB)
        unsigned int fEventStatus;
        float fPointingErrorX;                    //!< [deg]
        float fPointingErrorY;                    //!< [deg]
        unsigned int fMeanPointingErrorN;
        double fMeanPointingErrorX;               //!< [deg]
        double fMeanPointingErrorY;               //!< [deg]
        unsigned int fNEventsWithNoDBPointing;


        VPointingDB *fPointingDB;
        float fTelAzimuthDB;
        float fTelElevationDB;

        TTree *fPointingTree;

        double degrad;

        void fillPointingTree();
        void initializePointingTree();
        void reset();
        void updatePointingError( int, double );

    public:
        VSkyCoordinates( bool bReset = true, unsigned int iTelID = 0 );
        VSkyCoordinates( unsigned int itelID );
        ~VSkyCoordinates() {}
        double adjustAzimuthToRange( double );
        void   derotateCoords( double i_UTC, double i_xin, double i_yin, double & i_xout, double & i_yout);
        double getDerotationAngle(double i_UTC);
        void   getEquatorialCoordinates( int MJD, double time, double az, double ze, double &dec, double &ra );
        double getHourAngle(double i_UTC);
        double getSidereal(double i_UTC);
        void   getHorizonCoordinates( int MJD, double time, double dec, double ra, double &az, double &ze );
        string getTargetName();
        void   getRotatedShowerDirection( double y, double x, double &rze, double &raz );
        void   getRotatedShowerDirection( double ze, double az, double y, double x, double &rze, double &raz );
        void   getDerotatedShowerDirection( double ze, double az, float &y, float &x, double rze, double raz );
        void   getDifferenceInCameraCoordinates( double tel_ze, double tel_az, double shower_ze,  double shower_az, float &x, float &y );
        void   getDifferenceInCameraCoordinates( double tel_ze, double tel_az, double shower_ze,  double shower_az, float &x, float &y, float &z );
        float  getPointingErrorX();
        float  getPointingErrorY();
        VTargets* getTarget() { return fTarget; }
        double getTargetDec() { return fTargetDec * 180./TMath::Pi(); }
        double getTargetRA() { return fTargetRA * 180./TMath::Pi(); }
        double getTargetElevation() { return fTargetElevation; }
        double getTargetAzimuth() { return fTargetAzimuth; }
        double getTelAzimuth();
        double getTelElevation();
        double getTelDec() { return fTelDec * 180./TMath::Pi(); }
        double getTelRA() { return fTelRA * 180./TMath::Pi(); }
        double getTelLatitude() { return fObsLatitude*45./atan(1.); }
        double getTelLongitude() { return fObsLongitude*45./atan(1.); }
        void   getTelPointing( int MJD, double time, double &el, double &az );
        void   getPointingErrorFromDB( int irun, string iTCorrections, string iVPMDirectory, bool iVPMDB );
        unsigned int getTelID() { return fTelID; }
        double getWobbleNorth() { return fWobbleNorth; }
        double getWobbleEast() { return fWobbleEast; }
        bool   isSet() { return fSet; }
        void   rotateCoords( int i_mjd, double i_seconds, double i_xin, double i_yin, double & i_xout, double & i_yout);
        void   setMC() { fMC = true; }
	void   setObservatory( double iLongitude = 0., double iLatitude = 0. );
        bool   setPointingOffset( double i_raOff, double i_decOff );
        bool   setTarget( string iTargetName );
        bool   setTarget( double iDec, double iRA );
        void   setTargetName( string iTargetName ) { fTargetName = iTargetName; }
        void   precessTarget (int iMJD);
        bool   isPrecessed(){return fPrecessed;}
        void   setTelElevation( double );         //!< set telescope elevation (for MC)
        void   setTelAzimuth( double );           //!< set telescope azimuth (for MC)
        void   setTelPointing( int MJD, double time );
        void   setTelPointing( int MJD, double time, bool iGetPointingError );
                                                  //!< set telescope position ([deg])
        void   setTelPosition( double iLat, double iLong );
        void   setPointingError( double, double );//!< Pointing error [deg]
        void   setWobbleOffset( double iWobbleNorth, double iWobbleEast );
        void   terminate( bool i_IsMC = false );
};
#endif
