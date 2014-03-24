/*! \class VTarget
    \brief list of potential targets

    NOTE:    THIS CLASS SHOULD NOT BE USED ANYMORE
             COMPLETELY OUTDATED

    \author
    Gernot Maier

*/

#include "VTargets.h"

VTargets::VTargets()
{
	fIndexSelected = 9999;
	
	fTargetCode.push_back( "LS" );
	fTargetName.push_back( "laser" );
	fRA2000.push_back( "05:34:31.9" );
	fDec2000.push_back( "22:00:52" );
	fTargetCode.push_back( "CR" );
	fTargetName.push_back( "crab" );
	fRA2000.push_back( "05:34:31.9" );
	fDec2000.push_back( "22:00:52" );
	convertCoordinates();
}


void VTargets::convertCoordinates()
{
	for( unsigned int i = 0; i < fRA2000.size(); i++ )
	{
		fRA2000rad.push_back( getRArad( fRA2000[i] ) );
	}
	for( unsigned int i = 0; i < fDec2000.size(); i++ )
	{
		fDec2000rad.push_back( getDecrad( fDec2000[i] ) );
	}
}


double VTargets::getRArad( string iRA )
{
	double iH;
	double iM;
	double iS;
	
	double dRA;
	
	iH = atof( iRA.substr( 0, iRA.find( ":" ) ).c_str() );
	iM = atof( iRA.substr( iRA.find( ":" ) + 1, 2 ).c_str() );
	iS = atof( iRA.substr( iRA.rfind( ":" ) + 1, iRA.size() ).c_str() );
	
	dRA  = 15.*( iH + iM / 60. + iS / 3600. );
	
	dRA *= atan( 1. ) / 45.;
	
	return dRA;
}


double VTargets::getDecrad( string iDec )
{
	double dDec;
	double iM;
	double iS;
	
	dDec = atof( iDec.substr( 0, iDec.find( ":" ) ).c_str() );
	iM = atof( iDec.substr( iDec.find( ":" ) + 1, 2 ).c_str() );
	iS = atof( iDec.substr( iDec.rfind( ":" ) + 1, iDec.size() ).c_str() );
	
	if( dDec < 0. )
	{
		dDec = dDec - iM / 60. - iS / 3600.;
	}
	else
	{
		dDec = dDec + iM / 60. + iS / 3600.;
	}
	
	dDec *= atan( 1. ) / 45.;
	
	return dDec;
}


void VTargets::printTargets()
{
	cout << "Target list: " << endl << endl;
	for( unsigned int i = 0; i < fTargetCode.size(); i++ )
	{
		cout << i << "\t";
		cout << fTargetName[i] << "\t";
		if( fTargetName[i].size() < 8 )
		{
			cout << "\t";
		}
		cout << fDec2000[i] << "\t";
		cout << fRA2000[i] << "\t";
		cout << setprecision( 4 ) << showpoint;
		cout << showpos <<  fDec2000rad[i] * 45. / atan( 1. ) << "\t";
		cout << noshowpos;
		cout << fRA2000rad[i] * 45. / atan( 1. ) << "\t";
		cout << endl;
		
	}
	cout << endl;
}


bool VTargets::selectTargetbyName( string iTargetName )
{
	iTargetName = lowerCase( iTargetName );
	
	fIndexSelected = 9999;
	for( unsigned int i = 0; i < fTargetName.size(); i++ )
	{
		if( fTargetName[i] == iTargetName )
		{
			fIndexSelected = i;
			break;
		}
	}
	if( fIndexSelected  == 9999 )
	{
		cout << "VTargets::selectTargetbyName error: no target found with code " << iTargetName << " (" << iTargetName.size() << ")" << endl;
		return false;
	}
	return true;
}


bool VTargets::selectTargetbyCode( string iTargetCode )
{
	fIndexSelected = 9999;
	for( unsigned int i = 0; i < fTargetCode.size(); i++ )
	{
		if( fTargetCode[i] == iTargetCode )
		{
			fIndexSelected = i;
			break;
		}
	}
	if( fIndexSelected  == 9999 )
	{
		cout << "VTargets::selectTargetbyCode error: no target found with code " << iTargetCode << endl;
		return false;
	}
	return true;
}


bool VTargets::readTargets( string iTargetFile )
{
	iTargetFile = "";
	return true;
}


string VTargets::getTargetCode()
{
	if( fIndexSelected == 9999 || fTargetCode.size() >= fIndexSelected )
	{
		return "no target selected";
	}
	return fTargetCode[fIndexSelected];
}


string VTargets::getTargetName()
{
	if( fIndexSelected == 9999 || fTargetName.size() >= fIndexSelected )
	{
		return "no target selected";
	}
	return fTargetName[fIndexSelected];
}


double VTargets::getTargetDecJ2000()
{
	if( fIndexSelected == 9999 || fDec2000rad.size() <= fIndexSelected )
	{
		return -999.;
	}
	return fDec2000rad[fIndexSelected];
}


double VTargets::getTargetRAJ2000()
{
	if( fIndexSelected == 9999 || fRA2000rad.size() <= fIndexSelected )
	{
		return -999.;
	}
	return fRA2000rad[fIndexSelected];
}


//! Make a lowercase copy of s:
string VTargets::lowerCase( string& s )
{
	char* buf = new char[s.length()];
	s.copy( buf, s.length() );
	for( unsigned int i = 0; i < s.length(); i++ )
	{
		buf[i] = tolower( buf[i] );
	}
	string r( buf, s.length() );
	delete [] buf;
	return r;
}


void VTargets::setTarget( string iname, double iRaJ2000, double iDecJ2000 )
{
	fTargetCode.push_back( iname );
	fTargetName.push_back( iname );
	fRA2000.push_back( "not set" );
	fDec2000.push_back( "not set" );
	fRA2000rad.push_back( iRaJ2000 * atan( 1. ) / 45. );
	fDec2000rad.push_back( iDecJ2000 * atan( 1. ) / 45. );
	fIndexSelected = fTargetCode.size() - 1;
}

