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
	bool   fUseDB;                            //!< uses DB to calculate pointing directions

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

// telescope orientation
        double fTelAzimuth;                       //!< [deg]  return value to be used in the analysis
        double fTelElevation;                     //!< [deg]  return value to be used in the analysis
	float  fTelAzimuthCalculated;             //!< [deg]  elevation from source coordinates
	float  fTelElevationCalculated;           //!< [deg]  elevation from source coordinates
        float  fTelAzimuthDB;                     //!< [deg]  azimuth from VTS DB (from positioner or pointing monitor)
        float  fTelElevationDB;                   //!< [deg]  elevation from VTS DB (from positioner or pointing monitor)

        double fObsLatitude;                      //!< [rad]
        double fObsLongitude;                     //!< [rad]

        unsigned int fPointingType;               //!< 0: pointing calculated from source coordinates (+wobble offsets)
	                                          //!< 1: pointing calculated from source coordinates (+wobble offsets), added error from command line, 
	                                          //!< 2: read T-Point corrected positioner data from VERITAS DB
						  //!< 3: read raw positioner data from VERITAS DB and apply tracking corrections
						  //!< 4: from pointing monitor (text file)
						  //!< 5: from pointing monitor (DB)
        unsigned int fEventStatus;
        float fPointingErrorX;                    //!< [deg]
        float fPointingErrorY;                    //!< [deg]
        unsigned int fMeanPointingErrorN;
        double fMeanPointingErrorX;               //!< [deg]
        double fMeanPointingErrorY;               //!< [deg]
	double fMeanPointingDistance;             //!< [deg]
        unsigned int fNEventsWithNoDBPointing;


        VPointingDB *fPointingDB;

        TTree *fPointingTree;

        void fillPointingTree();
        void initializePointingTree();
        void reset();
        bool updatePointingfromDB( int, double );

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
	unsigned int getPointingType() { return fPointingType; }
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
        void   getPointingFromDB( int irun, string iTCorrections, string iVPMDirectory, bool iVPMDB );
        unsigned int getTelID() { return fTelID; }
        double getWobbleNorth() { return fWobbleNorth; }
        double getWobbleEast() { return fWobbleEast; }
        bool   isPrecessed(){return fPrecessed;}
        bool   isSet() { return fSet; }
        void   precessTarget (int iMJD);
        void   rotateCoords( int i_mjd, double i_seconds, double i_xin, double i_yin, double & i_xout, double & i_yout);
        void   setMC() { fMC = true; }
	void   setObservatory( double iLongitude_deg = 0., double iLatitude_deg = 0. );
        bool   setPointingOffset( double i_raOff, double i_decOff );
        bool   setTarget( string iTargetName );
        bool   setTarget( double iDec, double iRA );
        void   setTargetName( string iTargetName ) { fTargetName = iTargetName; }
        void   setTelElevation( double );         //!< set telescope elevation (for MC)
        void   setTelAzimuth( double );           //!< set telescope azimuth (for MC)
        void   setTelPointing( int MJD, double time, bool iUseDB = false, bool iFillPointingTree = false );
        void   setPointingError( double, double );//!< Pointing error [deg]
        void   setWobbleOffset( double iWobbleNorth, double iWobbleEast );
        void   terminate( bool i_IsMC = false );
};
#endif
