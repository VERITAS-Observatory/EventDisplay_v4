/*! \class VSkyCoordinates
    \brief get telescope pointing direction

    Revision $Id: VSkyCoordinates.cpp,v 1.19.2.1.4.14.12.2.6.2.2.6.10.1.2.1 2010/11/09 14:38:06 gmaier Exp $

    \author
       Gernot Maier
*/

#include "VSkyCoordinates.h"

VSkyCoordinates::VSkyCoordinates( bool bReset, unsigned int iTelID )
{
    fTelID = iTelID;
    fTarget = 0;
    fPointingTree = 0;
    fPointingDB = 0;

    if( bReset ) reset();
    setObservatory();
}


VSkyCoordinates::VSkyCoordinates( unsigned int iTelID )
{
    fTelID = iTelID;
    fTarget = 0;
    fPointingTree = 0;
    fPointingDB = 0;

    reset();
    setObservatory();

    initializePointingTree();
}

void VSkyCoordinates::setObservatory( double iLongitude, double iLatitude )
{
    fObsLatitude  = iLatitude * TMath::DegToRad();
    fObsLongitude = iLongitude * TMath::DegToRad();
}

void VSkyCoordinates::reset()
{
    fMC = false;
    fSet = false;
    fPrecessed = false;
    fWobbleSet = false;
    fUseDB = false;

    if( !fTarget ) fTarget = new VTargets();

    fPointingType = 0;
    fTelDec = 0.;
    fTelRA  = 0.;
    fTelDec_deg = 0.;
    fTelRA_deg  = 0.;
    fTargetDec = 0.;
    fTargetRA  = 0.;
    fTelAzimuth = 0.;
    fTelElevation = 0.;
    fTelAzimuthCalculated = 0.;
    fTelElevationCalculated = 0.;
    fTelAzimuthDB = 0.;
    fTelElevationDB = 0.;

    fTargetAzimuth = 0.;
    fTargetElevation = 0.;
    fWobbleNorth = 0.;
    fWobbleEast = 0.;

    fEventStatus = 0;
    fPointingErrorX = 0.;
    fPointingErrorY = 0.;
    fMeanPointingErrorN = 0;
    fMeanPointingErrorX = 0.;
    fMeanPointingErrorY = 0.;
    fMeanPointingDistance = 0.;
    fNEventsWithNoDBPointing = 0;

    fMJD = 0;
    fTime = 0.;
    fEventStatus = 0;

    fPointingDB = 0;
}



void VSkyCoordinates::setTelElevation( double iEl )
{
    fTelElevation = iEl;
}


void VSkyCoordinates::setTelAzimuth( double iAz )
{
    fTelAzimuth = iAz;
}

double VSkyCoordinates::getTelElevation()
{
    return fTelElevation;
}


double VSkyCoordinates::getTelAzimuth()
{
    return fTelAzimuth;
}


void VSkyCoordinates::precessTarget( int iMJD )
{
    if( !fPrecessed )
    {
        cout << "---------------------------------------------------------------------------------------------------------" << endl;
        cout << "Pointing telescope " << fTelID+1 << endl;
        cout << "\tPrecessing target ( " << getTargetName() << " ) from J2000 to MJD " << iMJD << endl;
        cout << "\tJ2000   \t\t RA=" << setprecision( 6 ) << fTargetRA*TMath::RadToDeg() << " dec=" << fTargetDec*TMath::RadToDeg() << endl;

        double ofy = VSkyCoordinatesUtilities::precessTarget( iMJD, fTargetRA, fTargetDec );
	cout << "\tMJD " << iMJD << " (" << setprecision( 6 ) << ofy << ")";
	cout << "\t RA=" << fTargetRA*TMath::RadToDeg() << " dec=" << fTargetDec*TMath::RadToDeg() << endl;
        VSkyCoordinatesUtilities::precessTarget( iMJD, fTelRA, fTelDec);

        fTelRA_deg = fTelRA * TMath::RadToDeg();
        fTelDec_deg = fTelDec * TMath::RadToDeg();
        fPrecessed = true;
    }
}


/*!
    \param iDec  [deg]
    \param iRA   [deg]
*/
bool VSkyCoordinates::setTarget( double iDec, double iRA )
{
    fTargetDec = iDec/TMath::RadToDeg();
    fTargetRA  = iRA/TMath::RadToDeg();
    fTelDec = iDec/TMath::RadToDeg();
    fTelRA  = iRA/TMath::RadToDeg();
    fTelRA_deg = iRA;
    fTelDec_deg = iDec;

    fSet = true;

    return true;
}


string VSkyCoordinates::getTargetName()
{
    return fTargetName;
}


bool VSkyCoordinates::setTarget( string iTargetName )
{
    if( fTarget && fTarget->selectTargetbyName( iTargetName ) )
    {
        fTargetName = iTargetName;
        fTargetDec = fTarget->getTargetDec();
        fTargetRA  = fTarget->getTargetRA();
        fTelRA = fTargetRA;
        fTelDec = fTargetDec;
        fTelRA_deg = fTelRA * TMath::RadToDeg();
        fTelDec_deg = fTelDec * TMath::RadToDeg();
        fSet = true;
    }
    else
    {
        cout << "VSkyCoordinates::setTarget invalid target: " << iTargetName << endl;
        return false;
    }
    return true;
}


/* 

    calculate azimuth and elevation of telescope pointing direction
    calculate azimuth and elevation of target

 */
void VSkyCoordinates::setTelPointing( int MJD, double time, bool iUseDB, bool iFillPointingTree )
{
    fMJD = (unsigned int)MJD;
    fTime = time;
    fUseDB = iUseDB;

    double iTime = 0.;
    double ha = 0.;
    double iSid = 0.;
    double az = 0.;
    double el = 0.;
// convert time to fraction of a day
    iTime = time / 86400.;
// get Greenwich sideral time
    iSid = slaGmsta( (double)MJD, iTime );
// calculate local sideral time
    iSid = iSid - fObsLongitude;
// calculate hour angle
    ha = slaDranrm( iSid - fTelRA );
// get horizontal coordinates
    slaDe2h( ha, fTelDec, fObsLatitude, &az, &el );
// from [rad] to [deg]
    el *= TMath::RadToDeg();
    az *= TMath::RadToDeg();

// telescope elevation/azimuth calculated from source coordinates and time
    fTelAzimuthCalculated   = (float)az;
    fTelElevationCalculated = (float)el;
// telescope elevation/azimuth calculated from VERITAS DB entries
    if( fUseDB )
    {
        updatePointingfromDB( fMJD, fTime );
    }
// calulation from source should always be successful
    else
    {
       fEventStatus = 1;
    }

// now set the global elevation/azimuth to be used for the analysis
    if( fUseDB && fPointingType > 1 )
    {
       fTelElevation = fTelElevationDB;
       fTelAzimuth   = fTelAzimuthDB;
    }
    else
    {
       fTelElevation = fTelElevationCalculated;
       fTelAzimuth   = fTelAzimuthCalculated;
    }

//////////////////////////////////////////////////
// set target azimuth/elevation

// calculate hour angle
    ha = slaDranrm( iSid - fTargetRA );
// get horizontal coordinates
    fTargetAzimuth = 0.;
    fTargetElevation = 0.;
    slaDe2h( ha, fTargetDec, fObsLatitude, &fTargetAzimuth, &fTargetElevation );
    fTargetAzimuth   *= TMath::RadToDeg();
    fTargetElevation *= TMath::RadToDeg();

// fill pointing tree
    if( iFillPointingTree ) fillPointingTree();

}



void VSkyCoordinates::getEquatorialCoordinates( int MJD, double time, double az, double ze, double &dec, double &ra )
{
    if( fMC )
    {
        dec = fTelDec;
        ra  = fTelRA;
        return;
    }
    double ha = 0.;
// transform coordinates
    slaDh2e( az/TMath::RadToDeg(), (90.-ze)/TMath::RadToDeg(), fObsLatitude, &ha, &dec );
// convert hour angle into ra
    double iTime = 0.;
    double iSid = 0.;
// convert time to fraction of a day
    iTime = time / 86400.;
// get Greenwich sideral time
    iSid = slaGmsta( (double)MJD, iTime );
// calculate local sideral time
    iSid = iSid - fObsLongitude;
// calculate right ascension
    ra = slaDranrm( iSid - ha );

    dec *= TMath::RadToDeg();
    ra  *= TMath::RadToDeg();
}

void VSkyCoordinates::getHorizonCoordinates( int MJD, double time, double dec, double ra, double &az, double &ze )
{
    if( fMC )
    {
        az = fTelAzimuth;
        ze = 90. - fTelElevation;
	return;
    }
    ra /= TMath::RadToDeg();
    dec /= TMath::RadToDeg();

// convert ra into hour angle
    double ha = 0.;
    double iTime = 0.;
    double iSid = 0.;
// convert time to fraction of a day
    iTime = time / 86400.;
// get Greenwich sideral time
    iSid = slaGmsta( (double)MJD, iTime );
// calculate local sideral time
    iSid = iSid - fObsLongitude;
// calculate right ascension
    ha = slaDranrm( iSid - ra );
// calculate equatorial coordinates
    double el = 0.;
    slaDe2h( ha, dec, fObsLatitude, &az, &el );
    el *= TMath::RadToDeg();

    ze = 90.-el;
    az *= TMath::RadToDeg();
}


bool VSkyCoordinates::setPointingOffset( double i_raOff, double i_decOff )
{
    fTelRA  = fTargetRA + i_raOff/TMath::RadToDeg();
    fTelDec = fTargetDec + i_decOff/TMath::RadToDeg();
    fTelRA_deg = fTelRA * TMath::RadToDeg();
    fTelDec_deg = fTelDec * TMath::RadToDeg();

    return true;
}


/*!
     input and output in [deg]
*/

void VSkyCoordinates::getRotatedShowerDirection( double y, double x, double &rze, double &raz )
{
    getRotatedShowerDirection( 90.-fTelElevation, fTelAzimuth, y, x, rze, raz );
}


/*
    calculate shower direction from telescope pointing and reconstruction shower direction

     small angle approximation, assume small x,y (neglect z)
*/

void VSkyCoordinates::getRotatedShowerDirection( double ze, double az, double y, double x, double &rze, double &raz )
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


/*
     difference between to pointing directions in camera coordinates

     input coordinates in [deg]
*/
void VSkyCoordinates::getDifferenceInCameraCoordinates( double tel_ze, double tel_az, double shower_ze,  double shower_az, float &x, float &y )
{
    float z;
    getDifferenceInCameraCoordinates( tel_ze, tel_az, shower_ze, shower_az, x, y, z );
}


void VSkyCoordinates::getDerotatedShowerDirection( double ze, double az, float &y, float &x, double rze, double raz )
{
    getDifferenceInCameraCoordinates( ze, az, rze, raz, y, x );
}


void VSkyCoordinates::getDifferenceInCameraCoordinates( double tel_ze, double tel_az, 
                                                        double shower_ze,  double shower_az, 
							float &x, float &y, float &z )
{
// convert coordinates from [deg] to [rad]
    tel_az /= TMath::RadToDeg();
    shower_az /= TMath::RadToDeg();
    double tel_el = (90.-tel_ze)/TMath::RadToDeg();
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


double VSkyCoordinates::getSidereal(double i_UTC)
{
    return slaDranrm(slaGmst(i_UTC)-fObsLongitude);
}


double VSkyCoordinates::getHourAngle(double i_UTC)
{
    return slaDranrm(getSidereal(i_UTC)-fTelRA);
}


double VSkyCoordinates::getDerotationAngle(double i_UTC)
{
//! Returns the camera derotation angle in radians
    return -slaPa(getHourAngle(i_UTC),fTelDec,fObsLatitude);
}


void VSkyCoordinates::derotateCoords( double i_UTC, double i_xin, double i_yin, double & i_xout, double & i_yout)
{
    double i_theta=getDerotationAngle(i_UTC);
    i_xout=i_xin*cos(i_theta)+i_yin*sin(i_theta);
    i_yout=i_yin*cos(i_theta)-i_xin*sin(i_theta);
}


void VSkyCoordinates::rotateCoords( int i_mjd, double i_seconds, double i_xin, double i_yin, double & i_xout, double & i_yout)
{
    double i_UTC = VSkyCoordinatesUtilities::getUTC( i_mjd, i_seconds );
    double i_theta = -1. * getDerotationAngle(i_UTC);
    i_xout=i_xin*cos(i_theta)+i_yin*sin(i_theta);
    i_yout=i_yin*cos(i_theta)-i_xin*sin(i_theta);
}


/*!
 *
 *  should be called after precession, etc.
 *
 */
void VSkyCoordinates::setWobbleOffset( double iNorth, double iEast )
{
    fWobbleNorth = iNorth;
    fWobbleEast = iEast;

    if( !fWobbleSet )
    {
        double i_decDiff = 0.;
	double i_RADiff = 0.;
	VSkyCoordinatesUtilities::getWobbleOffsets( iNorth, iEast, fTargetDec*TMath::RadToDeg(), fTargetRA*TMath::RadToDeg(), i_decDiff, i_RADiff );
	if( i_RADiff < -180. ) i_RADiff += 360.;

	fTelRA  = fTargetRA + i_RADiff/TMath::RadToDeg();
	fTelDec = fTargetDec + i_decDiff/TMath::RadToDeg();

        cout << "\tWobble mode, telescope " << fTelID+1 << " pointing to (ra,dec) = (" << fTelRA*TMath::RadToDeg() << ", " << fTelDec*TMath::RadToDeg() << ")";
        cout << ", (delta ra, delta dec) = (" << i_RADiff << ", " << i_decDiff << ")";
        cout << endl;

        fWobbleSet = true;
    }
    else
    {
        cout << "VSkyCoordinates::setWobbleOffset warning, wobble offsets already set" << endl;
    }
    fTelRA_deg = fTelRA * TMath::RadToDeg();
    fTelDec_deg = fTelDec * TMath::RadToDeg();
    cout << "---------------------------------------------------------------------------------------------------------" << endl;
}

/*

   return pointing error given by user

*/
float VSkyCoordinates::getPointingErrorX()
{
    if( fPointingType == 1 ) return fPointingErrorX;

    return 0.;
}

/*

   return pointing error given by user

*/
float VSkyCoordinates::getPointingErrorY()
{
    if( fPointingType == 1 ) return fPointingErrorY;

    return 0.;
}


void VSkyCoordinates::getPointingFromDB( int irun, string iTCorrection, string iVPMDirectory, bool iVPMDB  )
{
    fPointingType = 2;
    if( iVPMDB == true )                fPointingType = 5;    // read VPM data from VERITAS DB
    else if( iVPMDirectory.size() > 0 ) fPointingType = 4;    // read VPM data from a text file
    else if( iTCorrection.size() > 0 )  fPointingType = 3;    // read raw positioner data from VERITAS DB and apply tracking corrections
    else                                fPointingType = 2;    // read T-Point corrected positioner data from VERITAS DB

#ifdef RUNWITHDB
    fPointingDB = new VPointingDB( fTelID, irun, iTCorrection, iVPMDirectory, iVPMDB );
    if( !fPointingDB->isGood() )
    {
        cout << endl;
        cout << "FATAL ERROR: cannot connect to VERITAS database" << endl;
        cout << "exiting..." << endl;
        exit( 0 );
    }
    fPointingDB->setObservatory( fObsLongitude*TMath::RadToDeg(), fObsLatitude*TMath::RadToDeg() );          // work in [deg]
#else
    fPointingDB = 0;
#endif
}

/*

   set an artifical pointing error (e.g. from command line)

*/
void VSkyCoordinates::setPointingError( double iX, double iY )
{
    fPointingType = 1;
    fPointingErrorX = iX;
    fPointingErrorY = iY;

    fMeanPointingErrorN = 1;
    fMeanPointingErrorX = fPointingErrorX;
    fMeanPointingErrorY = fPointingErrorY;
    fMeanPointingDistance = sqrt( fPointingErrorX*fPointingErrorX + fPointingErrorY*fPointingErrorY );
}

/*
   
   update pointing from VERITAS database

*/
bool VSkyCoordinates::updatePointingfromDB( int MJD, double iTime )
{
// do something if we read stuff from the db
    if( fPointingDB )
    {
        fPointingDB->updatePointing( MJD, iTime );

// telescope pointings
        fTelAzimuthDB   = fPointingDB->getTelAzimuthDB();
        fTelElevationDB = fPointingDB->getTelElevationDB();
        fEventStatus    = fPointingDB->getEventStatus();

        if( fEventStatus != 3 )
        {
// calculate pointing error in camera coordinates (using slalib)
            double iPx = 0.;
	    double iPy = 0.;
            int j = 0;
            slaDs2tp( fTelAzimuthCalculated/TMath::RadToDeg(), fTelElevationCalculated/TMath::RadToDeg(), 
	              fTelAzimuthDB/TMath::RadToDeg(), fTelElevationDB/TMath::RadToDeg(),
		      &iPx, &iPy, &j );
            if( j == 0 )
            {
// azimuth from North to East
                fPointingErrorX = iPx*TMath::RadToDeg();
// evndisp camera is upside down
                fPointingErrorY = iPy*TMath::RadToDeg();
            }
// star not on tangent plane
            else
            {
                fPointingErrorX = 0.;
                fPointingErrorY = 0.;
                fEventStatus = 4;
            }
        }
        else
        {
            fPointingErrorX = 0.;
            fPointingErrorY = 0.;
            fNEventsWithNoDBPointing++;
        }
        fMeanPointingErrorN++;
        fMeanPointingErrorX += fPointingErrorX;
        fMeanPointingErrorY += fPointingErrorY;
	fMeanPointingDistance += sqrt( fPointingErrorX*fPointingErrorX + fPointingErrorY*fPointingErrorY );
    }
    if( fEventStatus != 3 && fEventStatus != 4 ) return true;

    return false;
}

/*

   mainly check that the different information about pointing matches

   write results to disk

*/
void VSkyCoordinates::terminate( bool i_isMC )
{
// don't do anything for MC
    if( i_isMC ) return;

    cout << "\t mean pointing mismatch between eventdisplay and DB for telescope " << getTelID()+1 << ":  (x,y,r) [deg] ";
    if( fMeanPointingErrorN > 0 )
    {
        cout << fMeanPointingErrorX/(double)fMeanPointingErrorN << ", " << fMeanPointingErrorY/(double)fMeanPointingErrorN;
	cout << ", " << fMeanPointingDistance/(double)fMeanPointingErrorN;
    }
    else                          cout << "0., 0., 0.";
    if( fNEventsWithNoDBPointing > 0 ) cout << ", number of events with no pointing information from the database: " << fNEventsWithNoDBPointing;
    cout << endl;

    if( fMeanPointingErrorN > 0 && (fMeanPointingDistance/(double)fMeanPointingErrorN > 0.1) )
    {
        cout << "WARNING: LARGE MISMATCH BETWEEN EVENTDISPLAY AND DB POINTING DATA FOR TELESCOPE " << getTelID()+1 << endl;
    }

//  write results to disk
    if( fPointingDB )
    {
        TTree *t = fPointingDB->getTreePointingDB();
        if( t ) t->Write();
        fPointingDB->terminate();
    }
    if( fPointingTree ) fPointingTree->Write();
}


void VSkyCoordinates::initializePointingTree()
{
    char hname[200];
    char htitle[200];

    sprintf( hname, "pointing_%d", getTelID()+1 );
    sprintf( htitle, "pointing (Telescope %d)", getTelID()+1 );
    fPointingTree = new TTree( hname, htitle );
    fPointingTree->Branch( "MJD", &fMJD, "MJD/i" );
    fPointingTree->Branch( "Time", &fTime, "Time/D" );
    fPointingTree->Branch( "TargetAzimuth", &fTargetAzimuth, "TargetAzimuth/D" );
    fPointingTree->Branch( "TargetElevation", &fTargetElevation, "TargetElevation/D" );
    fPointingTree->Branch( "TelAzimuth", &fTelAzimuth, "TelAzimuth/D" );
    fPointingTree->Branch( "TelElevation", &fTelElevation, "TelElevation/D" );
    fPointingTree->Branch( "PointingType", &fPointingType, "fPointingType/i" );
    fPointingTree->Branch( "TelAzimuthCalculated", &fTelAzimuthCalculated, "TelAzimuthCalculated/F" );
    fPointingTree->Branch( "TelElevationCalculated", &fTelElevationCalculated, "TelElevationCalculated/F" );
    fPointingTree->Branch( "TelAzimuthDB", &fTelAzimuthDB, "TelAzimuthDB/F" );
    fPointingTree->Branch( "TelElevationDB", &fTelElevationDB, "TelElevationDB/F" );
    fPointingTree->Branch( "PointingErrorX", &fPointingErrorX, "PointingErrorX/F" );
    fPointingTree->Branch( "PointingErrorY", &fPointingErrorY, "PointingErrorY/F" );
    fPointingTree->Branch( "EventStatus", &fEventStatus, "EventStatus/i" );
}

void VSkyCoordinates::fillPointingTree()
{
    if( fPointingTree ) fPointingTree->Fill();
}


double VSkyCoordinates::adjustAzimuthToRange( double az )
{
    return slaDranrm( az/TMath::RadToDeg()) * TMath::RadToDeg();
}
