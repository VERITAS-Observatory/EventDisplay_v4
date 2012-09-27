/*! \class VSkyCoordinates
    \brief get telescope pointing direction

    Revision $Id: VSkyCoordinates.cpp,v 1.19.2.1.4.14.12.2.6.2.2.6.10.1.2.1 2010/11/09 14:38:06 gmaier Exp $

    \author
       Gernot Maier
*/

#include "VSkyCoordinates.h"

VSkyCoordinates::VSkyCoordinates()
{
    fStarCatalogue = 0;

    reset();
    setObservatory();
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

    fTargetAzimuth = 0.;
    fTargetElevation = 0.;
    fWobbleNorth = 0.;
    fWobbleEast = 0.;

    fMJD = 0;
    fTime = 0.;
}

void VSkyCoordinates::precessTarget( int iMJD, unsigned int iTelID )
{
    if( !fPrecessed )
    {
        cout << "---------------------------------------------------------------------------------------------------------" << endl;
        cout << "Pointing telescope " << iTelID+1 << endl;
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

bool VSkyCoordinates::setTarget( string iTargetName )
{
    VTargets iTarget;
    if( iTarget.selectTargetbyName( iTargetName ) )
    {
        fTargetName = iTargetName;
        fTargetDec = iTarget.getTargetDecJ2000();
        fTargetRA  = iTarget.getTargetRAJ2000();
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
void VSkyCoordinates::updatePointing( int MJD, double time )
{
    fMJD = (unsigned int)MJD;
    fTime = time;

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
    fTelElevation = fTelElevationCalculated;
    fTelAzimuth   = fTelAzimuthCalculated;

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

bool VSkyCoordinates::setPointingOffset( double i_raOff, double i_decOff )
{
    fTelRA      = fTargetRA + i_raOff/TMath::RadToDeg();
    fTelDec     = fTargetDec + i_decOff/TMath::RadToDeg();
    fTelRA_deg  = fTelRA * TMath::RadToDeg();
    fTelDec_deg = fTelDec * TMath::RadToDeg();

    return true;
}


/*!
     input and output in [deg]
*/

void VSkyCoordinates::getRotatedShowerDirection( double y, double x, double &rze, double &raz )
{
    VSkyCoordinatesUtilities::getRotatedShowerDirection( 90.-fTelElevation, fTelAzimuth, y, x, rze, raz );
}


void VSkyCoordinates::getDerotatedShowerDirection( double ze, double az, float &y, float &x, double rze, double raz )
{
    float z = 0.;
    VSkyCoordinatesUtilities::getDifferenceInCameraCoordinates( ze, az, rze, raz, y, x, z );
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
void VSkyCoordinates::setWobbleOffset( double iNorth, double iEast, unsigned int iTelID )
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

        cout << "\tWobble mode, telescope " << iTelID+1 << " pointing to (ra,dec) = (" << fTelRA*TMath::RadToDeg() << ", " << fTelDec*TMath::RadToDeg() << ")";
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

bool VSkyCoordinates::initStarCatalogue( string iCatalogueName, double iMJD, double xmin, double xmax, double ymin, double ymax, 
                                         double iRASkyMapCentre, double iDecSkyMapCentre )
{
    if( !fStarCatalogue )
    {
       fStarCatalogue = new VStarCatalogue();
    }
    if( fStarCatalogue )
    {
        double i_x = 0.;
        double i_y = 0.;
        if( fabs( xmin ) > fabs( xmax ) ) i_x = fabs( xmin );
        else                              i_x = fabs( xmax );
        if( fabs( ymin ) > fabs( ymax ) ) i_y = fabs( ymin );
        else                              i_y = fabs( ymax );
        if( !fStarCatalogue->init( iMJD, iCatalogueName ) )
        {
            cout << "Error reading star catalogue: " << iCatalogueName << endl;
            cout << "exiting..." << endl;
            return false;
        }
        fStarCatalogue->setFOV( iRASkyMapCentre, iDecSkyMapCentre, i_x, i_y, true );
    }

    return true;
}
