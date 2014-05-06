/*! \class VSkyCoordinates
    \brief handle telescope pointing direction and target positions


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


VSkyCoordinates::~VSkyCoordinates()
{
	if( fStarCatalogue )
	{
		delete fStarCatalogue;
	}
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
	fTelDecJ2000 = 0.;
	fTelRAJ2000  = 0.;
	fTargetDec = 0.;
	fTargetRA  = 0.;
	fTargetDecJ2000 = 0.;
	fTargetRAJ2000  = 0.;
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
	
	fSupressStdoutText = false ;
}

void VSkyCoordinates::precessTarget( int iMJD, int iTelID )
{
	if( !fPrecessed )
	{
		if( !fSupressStdoutText )
		{
			cout << "---------------------------------------------------------------------------------------------------------" << endl;
		}
		if( iTelID >= 0 )
		{
			if( !fSupressStdoutText )
			{
				cout << "Pointing telescope " << iTelID + 1 << endl;
			}
		}
		else
		{
			cout << "Array pointing " << endl;
		}
		if( !fSupressStdoutText )
		{
			cout << "\tPrecessing target ( " << getTargetName() << " ) from J2000 to MJD " << iMJD << endl;
			// TEMP
			cout << "\tJ2000   \t\t RA=" << setprecision( 6 ) << fTargetRA* TMath::RadToDeg() << " dec=" << fTargetDec* TMath::RadToDeg() << endl;
		}
		// ENDTEMP
		// precess target coordinates
		double ofy = VSkyCoordinatesUtilities::precessTarget( iMJD, fTargetRA, fTargetDec );
		
		if( !fSupressStdoutText )
		{
			cout << "\tMJD " << iMJD << " (" << setprecision( 6 ) << ofy << ")";
			cout << "\t RA=" << fTargetRA* TMath::RadToDeg() << " dec=" << fTargetDec* TMath::RadToDeg() << endl;
		}
		// precess telescope coordinates
		VSkyCoordinatesUtilities::precessTarget( iMJD, fTelRA, fTelDec );
		
		fPrecessed = true;
	}
}


/*!

    set target coordinates in J2000

    \param iDec  [deg]
    \param iRA   [deg]
*/
bool VSkyCoordinates::setTargetJ2000( double iDec, double iRA )
{
	fTargetDecJ2000 = iDec / TMath::RadToDeg();
	fTargetRAJ2000  = iRA / TMath::RadToDeg();
	
	// unprecessed -> precess later in the analysis
	fTargetDec = iDec / TMath::RadToDeg();
	fTargetRA  = iRA / TMath::RadToDeg();
	
	fTelDecJ2000 = iDec / TMath::RadToDeg();
	fTelRAJ2000  = iRA / TMath::RadToDeg();
	// unprecessed -> precess later in the analysis
	fTelDec = iDec / TMath::RadToDeg();
	fTelRA  = iRA / TMath::RadToDeg();
	
	fSet = true;
	
	return true;
}

/*

    this shouldn't be used (or only if you know what you do)

*/
bool VSkyCoordinates::setTarget( string iTargetName )
{
	VTargets iTarget;
	if( iTarget.selectTargetbyName( iTargetName ) )
	{
		fTargetName = iTargetName;
		
		fTargetDecJ2000 = iTarget.getTargetDecJ2000();
		fTargetRAJ2000  = iTarget.getTargetRAJ2000();
		fTargetDec = iTarget.getTargetDecJ2000();
		fTargetRA  = iTarget.getTargetRAJ2000();
		fTelRA = fTargetRA;
		fTelDec = fTargetDec;
		fTelRAJ2000 = fTargetRA;
		fTelDecJ2000 = fTargetDec;
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
	fMJD = ( unsigned int )MJD;
	fTime = time;
	
	double iTime = 0.;
	double ha = 0.;
	double iSid = 0.;
	double az = 0.;
	double el = 0.;
	// convert time to fraction of a day
	iTime = time / 86400.;
	// get Greenwich sideral time
	iSid = slaGmsta( ( double )MJD, iTime );
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
	fTelAzimuthCalculated   = ( float )az;
	fTelElevationCalculated = ( float )el;
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

/*

    calculate right ascension / declination

    all angles (in/out) in [deg]

*/
void VSkyCoordinates::getEquatorialCoordinates( int MJD, double time, double az, double ze, double& dec, double& ra )
{
	if( fMC )
	{
		dec = fTelDec;
		ra  = fTelRA;
		return;
	}
	VSkyCoordinatesUtilities::getEquatorialCoordinates( MJD, time, az, ze, dec, ra );
}

/*

   add an offset in ra/dec

   this should happen before precession is applied

*/
bool VSkyCoordinates::setPointingOffset( double i_raOff_deg, double i_decOff_deg )
{
	if( isPrecessed() )
	{
		fTelRA = -9999.;
		fTelDec = -9999.;
		fTelRAJ2000 = -9999.;
		fTelDecJ2000 = -9999.;
		return false;
	}
	
	fTelRA      = fTargetRA + i_raOff_deg * TMath::DegToRad();
	fTelDec     = fTargetDec + i_decOff_deg * TMath::DegToRad();
	
	fTelRAJ2000      = fTargetRAJ2000 + i_raOff_deg * TMath::DegToRad();
	fTelDecJ2000     = fTargetDecJ2000 + i_decOff_deg * TMath::DegToRad();
	
	return true;
}


/*!
     input and output in [deg]
*/

void VSkyCoordinates::getRotatedShowerDirection( double y, double x, double& rze, double& raz )
{
	VSkyCoordinatesUtilities::getRotatedShowerDirection( 90. - fTelElevation, fTelAzimuth, y, x, rze, raz );
}


void VSkyCoordinates::getDerotatedShowerDirection( double ze, double az, float& y, float& x, double rze, double raz )
{
	float z = 0.;
	VSkyCoordinatesUtilities::getDifferenceInCameraCoordinates( ze, az, rze, raz, y, x, z );
}


double VSkyCoordinates::getSidereal( double i_UTC )
{
	return slaDranrm( slaGmst( i_UTC ) - fObsLongitude );
}


double VSkyCoordinates::getHourAngle( double i_UTC )
{
	return slaDranrm( getSidereal( i_UTC ) - fTelRA );
}

double VSkyCoordinates::getDerotationAngle( int iMJD, double iTime )
{
	return getDerotationAngle( VSkyCoordinatesUtilities::getUTC( iMJD, iTime ) );
}


double VSkyCoordinates::getDerotationAngle( double i_UTC )
{
	//! Returns the camera derotation angle in radians
	return -slaPa( getHourAngle( i_UTC ), fTelDec, fObsLatitude );
}

void VSkyCoordinates::derotateCoords( double i_UTC, double i_xin, double i_yin, double& i_xout, double& i_yout )
{
	double i_theta = getDerotationAngle( i_UTC );
	i_xout = i_xin * cos( i_theta ) + i_yin * sin( i_theta );
	i_yout = i_yin * cos( i_theta ) - i_xin * sin( i_theta );
}

void VSkyCoordinates::derotateCoords( int i_mjd, double i_seconds, double i_xin, double i_yin, double& i_xout, double& i_yout )
{
	double i_UTC = VSkyCoordinatesUtilities::getUTC( i_mjd, i_seconds );
	double i_theta = getDerotationAngle( i_UTC );
	i_xout = i_xin * cos( i_theta ) + i_yin * sin( i_theta );
	i_yout = i_yin * cos( i_theta ) - i_xin * sin( i_theta );
}

void VSkyCoordinates::rotateCoords( int i_mjd, double i_seconds, double i_xin, double i_yin, double& i_xout, double& i_yout )
{
	double i_UTC = VSkyCoordinatesUtilities::getUTC( i_mjd, i_seconds );
	double i_theta = -1. * getDerotationAngle( i_UTC );
	i_xout = i_xin * cos( i_theta ) + i_yin * sin( i_theta );
	i_yout = i_yin * cos( i_theta ) - i_xin * sin( i_theta );
}


/*!
 *
 *  should be called after precession, etc.
 *
 */
void VSkyCoordinates::setWobbleOffset( double iNorth, double iEast, int iTelID, int iMJD )
{
	fWobbleNorth = iNorth;
	fWobbleEast = iEast;
	
	if( !fWobbleSet )
	{
		double i_decDiff = 0.;
		double i_RADiff = 0.;
		VSkyCoordinatesUtilities::getWobbleOffset_in_RADec( iNorth, iEast, fTargetDec * TMath::RadToDeg(), fTargetRA * TMath::RadToDeg(), i_decDiff, i_RADiff );
		if( i_RADiff < -180. )
		{
			i_RADiff += 360.;
		}
		
		fTelRA  = fTargetRA + i_RADiff * TMath::DegToRad();
		fTelDec = fTargetDec + i_decDiff * TMath::DegToRad();
		
		if( iTelID >= 0 )
		{
			cout << "\tWobble mode, telescope " << iTelID + 1;
		}
		else
		{
			cout << "\tWobble mode, array ";
		}
		cout << " pointing to (ra,dec) = (" << fTelRA* TMath::RadToDeg() << ", " << fTelDec* TMath::RadToDeg() << ")";
		cout << ", (delta ra, delta dec) = (" << i_RADiff << ", " << i_decDiff << ")";
		cout << endl;
		
		// set J2000 telescope coordinates
		fTelRAJ2000 = fTelRA;
		fTelDecJ2000 = fTelDec;
		VSkyCoordinatesUtilities::precessTarget( 51544., fTelRAJ2000, fTelDecJ2000, iMJD );
		
		fWobbleSet = true;
	}
	else
	{
		cout << "VSkyCoordinates::setWobbleOffset warning, wobble offsets already set" << endl;
	}
	cout << "---------------------------------------------------------------------------------------------------------" << endl;
}

/*

   initial star catalogue and set FOV in catalogue

   expect J2000 coordinates

*/
bool VSkyCoordinates::initStarCatalogue( string iCatalogueName, double iMJD, double xmin, double xmax, double ymin, double ymax,
		double iRASkyMapCentre_J2000, double iDecSkyMapCentre_J2000 )
{
	if( !fStarCatalogue )
	{
		fStarCatalogue = new VStarCatalogue();
	}
	if( fStarCatalogue )
	{
		double i_x = 0.;
		double i_y = 0.;
		if( fabs( xmin ) > fabs( xmax ) )
		{
			i_x = fabs( xmin );
		}
		else
		{
			i_x = fabs( xmax );
		}
		if( fabs( ymin ) > fabs( ymax ) )
		{
			i_y = fabs( ymin );
		}
		else
		{
			i_y = fabs( ymax );
		}
		if( !fStarCatalogue->init( iMJD, iCatalogueName ) )
		{
			cout << "Error reading star catalogue: " << iCatalogueName << endl;
			cout << "exiting..." << endl;
			return false;
		}
		fStarCatalogue->setFOV( iRASkyMapCentre_J2000, iDecSkyMapCentre_J2000, i_x, i_y, true );
	}
	
	return true;
}
