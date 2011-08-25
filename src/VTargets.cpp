/*! \class VTarget
    \brief list of potential targets

    NOTE:    THIS CLASS SHOULD NOT BE USED ANYMORE

    \author
    Gernot Maier

    Revision $Id: VTargets.cpp,v 1.33.2.1.4.4.12.1.8.2.10.1 2010/03/08 07:39:53 gmaier Exp $
*/

#include "VTargets.h"

VTargets::VTargets()
{
    fIndexSelected = 9999;

    fTargetCode.push_back( "WY" );  fTargetName.push_back( "wy" );       fRA2000.push_back( "05:34:31.9" ); fDec2000.push_back( "22:00:52" );
    fTargetCode.push_back( "LS" );  fTargetName.push_back( "laser" );       fRA2000.push_back( "05:34:31.9" ); fDec2000.push_back( "22:00:52" );
    fTargetCode.push_back( "CR" );  fTargetName.push_back( "crab" );       fRA2000.push_back( "05:34:31.9" ); fDec2000.push_back( "22:00:52" );
    fTargetCode.push_back( "E03" ); fTargetName.push_back( "1es0033" );    fRA2000.push_back( "00:35:52.6" ); fDec2000.push_back( "59:50:05" );
    fTargetCode.push_back( "E2" );  fTargetName.push_back( "1es0120" );    fRA2000.push_back( "01:23:08.5" ); fDec2000.push_back( "34:20:48" );
    fTargetCode.push_back( "J02" ); fTargetName.push_back( "rgbj0214" );   fRA2000.push_back( "02:14:17.9" ); fDec2000.push_back( "51:44:52" );
    fTargetCode.push_back( "66A" ); fTargetName.push_back( "3c66a" );      fRA2000.push_back( "02:22:39.6" ); fDec2000.push_back( "43:02:08" );
    fTargetCode.push_back( "E0" );  fTargetName.push_back( "1es0229" );    fRA2000.push_back( "02:32:48.4" ); fDec2000.push_back( "20:17:16" );
    fTargetCode.push_back( "E47" ); fTargetName.push_back( "1es0314" );    fRA2000.push_back( "03:14:02.7" ); fDec2000.push_back( "24:44:33" );
    fTargetCode.push_back( "E03" ); fTargetName.push_back( "1es0323" );    fRA2000.push_back( "03:26:14.0" ); fDec2000.push_back( "02:25:15" );
    fTargetCode.push_back( "IC3" ); fTargetName.push_back( "ic342" );      fRA2000.push_back( "03:46:48.5" ); fDec2000.push_back( "68:05:46" );
    fTargetCode.push_back( "IC" );  fTargetName.push_back( "ic443" );      fRA2000.push_back( "06:17:05.3" ); fDec2000.push_back( "22:21:27" );
    fTargetCode.push_back( "E08" ); fTargetName.push_back( "1es0806" );    fRA2000.push_back( "08:09:49.1" ); fDec2000.push_back( "52:18:59" );
    fTargetCode.push_back( "E96" ); fTargetName.push_back( "1es1011" );    fRA2000.push_back( "10:15:04.1" ); fDec2000.push_back( "49:26:01" );
    fTargetCode.push_back( "M4" );  fTargetName.push_back( "mrk421" );     fRA2000.push_back( "11:04:27.3" ); fDec2000.push_back( "38:12:32" );
    fTargetCode.push_back( "M2" );  fTargetName.push_back( "mrk180" );     fRA2000.push_back( "11:36:26.4" ); fDec2000.push_back( "70:09:27" );
    fTargetCode.push_back( "R3" );  fTargetName.push_back( "rxj1136" );    fRA2000.push_back( "11:36:30.1" ); fDec2000.push_back( "67:37:04" );
    fTargetCode.push_back( "J37" ); fTargetName.push_back( "3egj1337" );   fRA2000.push_back( "13:35:00.0" ); fDec2000.push_back( "51:00:00" );
    fTargetCode.push_back( "H4" );  fTargetName.push_back( "h1426" );      fRA2000.push_back( "14:28:32.6" ); fDec2000.push_back( "42:40:21" );
    fTargetCode.push_back( "E15" ); fTargetName.push_back( "1es1553" );    fRA2000.push_back( "15:55:43.0" ); fDec2000.push_back( "11:11:24" );
    fTargetCode.push_back( "M5" );  fTargetName.push_back( "mrk501" );     fRA2000.push_back( "16:53:52.2" ); fDec2000.push_back( "39:45:36" );
    fTargetCode.push_back( "J17" ); fTargetName.push_back( "rgbj1725" );   fRA2000.push_back( "17:25:04.3" ); fDec2000.push_back( "11:52:15" );
    fTargetCode.push_back( "IZ" );  fTargetName.push_back( "1es1727" );    fRA2000.push_back( "17:28:18.6" ); fDec2000.push_back( "50:13:10" );
    fTargetCode.push_back( "E17" ); fTargetName.push_back( "1es1741" );    fRA2000.push_back( "17:43:57.8" ); fDec2000.push_back( "19:35:09" );
    fTargetCode.push_back( "E9" );  fTargetName.push_back( "1es1959" );    fRA2000.push_back( "19:59:59.9" ); fDec2000.push_back( "65:08:55" );
    fTargetCode.push_back( "OB2" ); fTargetName.push_back( "tev2032" );    fRA2000.push_back( "20:32:07.0" ); fDec2000.push_back( "41:30:30" );
    fTargetCode.push_back( "VR" );  fTargetName.push_back( "bllac" );      fRA2000.push_back( "22:02:43.3" ); fDec2000.push_back( "42:16:40" );
    fTargetCode.push_back( "3E" );  fTargetName.push_back( "1es2344" );    fRA2000.push_back( "23:47:04.8" ); fDec2000.push_back( "51:42:18" );
    fTargetCode.push_back( "RX" );  fTargetName.push_back( "rxj1117.1+2014" ); fRA2000.push_back( "11:17:06.2" ); fDec2000.push_back( "20:14:07" );
    fTargetCode.push_back( "G28" ); fTargetName.push_back( "g28.6-0.1" );  fRA2000.push_back( "18:43:55" );  fDec2000.push_back( "-03:53:00" );
    fTargetCode.push_back( "B24" ); fTargetName.push_back( "bl0224+014" ); fRA2000.push_back( "02:27:16.6" ); fDec2000.push_back( "02:01:58" );
    fTargetCode.push_back( "M87" ); fTargetName.push_back( "m87" ); fRA2000.push_back( "12:30:49.2" ); fDec2000.push_back( "12:23:28" );
    fTargetCode.push_back( "LSI" ); fTargetName.push_back( "lsi+61303" );  fRA2000.push_back( "02:40:31.67" ); fDec2000.push_back( "61:13:45.6" );
    fTargetCode.push_back( "M15" ); fTargetName.push_back( "m15" );        fRA2000.push_back( "21:29:58.38" ); fDec2000.push_back( "12:10:00.6" );
// intermediate point between M15 and 10m excess
    fTargetCode.push_back( "M15M" ); fTargetName.push_back( "m15_mid" );   fRA2000.push_back( "21:31:12.00" ); fDec2000.push_back( "12:24:25" );
// position of 10m excess
    fTargetCode.push_back( "M1510" ); fTargetName.push_back( "m15_10m" );  fRA2000.push_back( "21:32:27.00" ); fDec2000.push_back( "12:38:49" );
    fTargetCode.push_back("J29");     fTargetName.push_back( "psrj2229+6114"); fRA2000.push_back( "22:29:05.0" ); fDec2000.push_back( "61:14:13" );
    fTargetCode.push_back("J20");     fTargetName.push_back( "psrj2021+3651"); fRA2000.push_back( "20:21:04.5" ); fDec2000.push_back( "36:51:27" );
    fTargetCode.push_back("J20b");     fTargetName.push_back( "psrj2021"); fRA2000.push_back( "20:21:04.5" ); fDec2000.push_back( "36:51:27" );
    fTargetCode.push_back("TDB");     fTargetName.push_back( "tycho"); fRA2000.push_back( "00:25:08.1" ); fDec2000.push_back( "64:09:56" );
    fTargetCode.push_back("E06");     fTargetName.push_back( "1es0647+250"); fRA2000.push_back( "06:50:46.5" ); fDec2000.push_back( "25:03:00" );
    fTargetCode.push_back("E06b");    fTargetName.push_back( "1es0647"); fRA2000.push_back( "06:50:46.5" ); fDec2000.push_back( "25:03:00" );
    fTargetCode.push_back("E33");     fTargetName.push_back( "1es0033"); fRA2000.push_back( "00:35:52.63" ); fDec2000.push_back( "59:50:04.6" );
    fTargetCode.push_back("ZE");      fTargetName.push_back( "ZetaTau"); fRA2000.push_back( "05:37:38.6858" ); fDec2000.push_back( "21:08:33.177" );
    fTargetCode.push_back("3C58");    fTargetName.push_back( "3c58"); fRA2000.push_back( "02:05:38" ); fDec2000.push_back( "64:49:42" );
    fTargetCode.push_back("1ES1218"); fTargetName.push_back( "1es1218"); fRA2000.push_back( "12:21:21.941" ); fDec2000.push_back( "30:10:37.11" );
    fTargetCode.push_back("MON");     fTargetName.push_back( "monoceros"); fRA2000.push_back( "06:34:51.5" ); fDec2000.push_back( "05:56:05" );
    fTargetCode.push_back("3C279");   fTargetName.push_back( "3c279"); fRA2000.push_back( "12:56:11.17" ); fDec2000.push_back( "-05:47:21.53" );
    fTargetCode.push_back("Theta1OriC");   fTargetName.push_back( "theta1oric"); fRA2000.push_back( "05:35:16.5" ); fDec2000.push_back( "-05:23:23" );
    fTargetCode.push_back("GM");   fTargetName.push_back( "geminga"); fRA2000.push_back( "06:33:54.2" ); fDec2000.push_back( "+17:46:13" );
    fTargetCode.push_back("B06");   fTargetName.push_back( "psrb0656+14"); fRA2000.push_back( "06:59:48.1" ); fDec2000.push_back( "+14:14:22" );
    fTargetCode.push_back("UM");   fTargetName.push_back( "ursaminor"); fRA2000.push_back( "15:09:08" ); fDec2000.push_back( "67:13:21" );
    fTargetCode.push_back("DR");   fTargetName.push_back( "draco"); fRA2000.push_back( "17:20:13" ); fDec2000.push_back( "57:54:54" );
    fTargetCode.push_back("GRB");   fTargetName.push_back( "grb070311"); fRA2000.push_back( "05:50:08.21" ); fDec2000.push_back( "03:22:30.3" );
    fTargetCode.push_back("J16");   fTargetName.push_back( "1es1627"); fRA2000.push_back( "16:29:01.3" ); fDec2000.push_back( "40:08:00" );
    fTargetCode.push_back("t009");   fTargetName.push_back( "1es1101"); fRA2000.push_back( "11:03:37.6" ); fDec2000.push_back( "-23:29:30" );
    fTargetCode.push_back("t022");   fTargetName.push_back( "psrj1740"); fRA2000.push_back( "17:40:26.0" ); fDec2000.push_back( "+10:00:06" );
    fTargetCode.push_back("1E18");  fTargetName.push_back( "1es1118"); fRA2000.push_back( "11:20:47.5" ); fDec2000.push_back( "42:12:17" );
    fTargetCode.push_back("1E2" ); fTargetName.push_back( "1es1215"); fRA2000.push_back( "12:17:36" ); fDec2000.push_back( "30:07:00.6" );
    fTargetCode.push_back("R14" ); fTargetName.push_back( "rgbj1413+436"); fRA2000.push_back( "14:13:43.6" ); fDec2000.push_back( "43:39:44" );
    fTargetCode.push_back("SAX1" ); fTargetName.push_back( "saxj2103"); fRA2000.push_back( "21:03:36" ); fDec2000.push_back( "45:44:28" );
    fTargetCode.push_back("M82" ); fTargetName.push_back( "m82"); fRA2000.push_back( "09:55:52.2" ); fDec2000.push_back( "69:40:49" );
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
    iM = atof( iRA.substr( iRA.find( ":" )+1, 2 ).c_str() );
    iS = atof( iRA.substr( iRA.rfind( ":" )+1, iRA.size() ).c_str() );

    dRA  = 15.*( iH + iM/60. + iS/3600. );

    dRA *= atan(1.)/45.;

    return dRA;
}


double VTargets::getDecrad( string iDec )
{
    double dDec;
    double iM;
    double iS;

    dDec = atof( iDec.substr( 0, iDec.find( ":" ) ).c_str() );
    iM = atof( iDec.substr( iDec.find( ":" )+1, 2 ).c_str() );
    iS = atof( iDec.substr( iDec.rfind( ":" )+1, iDec.size() ).c_str() );

    if( dDec < 0. ) dDec = dDec - iM/60. - iS/3600.;
    else            dDec = dDec + iM/60. + iS/3600.;

    dDec *= atan(1.)/45.;

    return dDec;
}


void VTargets::printTargets()
{
    cout << "Target list: " << endl << endl;
    for( unsigned int i = 0; i < fTargetCode.size(); i++ )
    {
        cout << i << "\t";
//       cout << fTargetCode[i] << "\t";
        cout << fTargetName[i] << "\t";
        if( fTargetName[i].size() < 8 ) cout << "\t";
        cout << fDec2000[i] << "\t";
        cout << fRA2000[i] << "\t";
        cout << setprecision( 4 ) << showpoint;
        cout << showpos <<  fDec2000rad[i]*45./atan(1.) << "\t";
        cout << noshowpos;
        cout << fRA2000rad[i]*45./atan(1.) << "\t";
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
    return true;
}


string VTargets::getTargetCode()
{
    if( fIndexSelected == 9999 || fTargetCode.size() >= fIndexSelected ) return "no target selected";
    return fTargetCode[fIndexSelected];
}


string VTargets::getTargetName()
{
    if( fIndexSelected == 9999 || fTargetName.size() >= fIndexSelected ) return "no target selected";
    return fTargetName[fIndexSelected];
}


double VTargets::getTargetDec()
{
    if( fIndexSelected == 9999 || fDec2000rad.size() <= fIndexSelected ) return -999.;
    return fDec2000rad[fIndexSelected];
}


double VTargets::getTargetRA()
{
    if( fIndexSelected == 9999 || fRA2000rad.size() <= fIndexSelected ) return -999.;
    return fRA2000rad[fIndexSelected];
}


//! Make a lowercase copy of s:
string VTargets::lowerCase( string& s)
{
    char* buf = new char[s.length()];
    s.copy(buf, s.length());
    for( unsigned int i = 0; i < s.length(); i++) buf[i] = tolower(buf[i]);
    string r(buf, s.length());
    delete [] buf;
    return r;
}


void VTargets::setTarget( string iname, double iRaJ2000, double iDecJ2000 )
{
    fTargetCode.push_back( iname ); fTargetName.push_back( iname ); fRA2000.push_back( "not set" ); fDec2000.push_back( "not set" );
    fRA2000rad.push_back( iRaJ2000 * atan(1.)/45. );
    fDec2000rad.push_back( iDecJ2000 * atan(1.)/45. );
    fIndexSelected = fTargetCode.size() - 1;
}


double VTargets::getTargetShiftWest( double ira, double idec )
{
    double degra = atan(1.)/45.;

    double sep  = slaDsep(  getTargetRAJ2000(), getTargetDecJ2000(), ira * degra, idec * degra );
    double bear = slaDbear( getTargetRAJ2000(), getTargetDecJ2000(), ira * degra, idec * degra );

    double iShift = sep * sin( bear ) / degra;

    if( TMath::Abs( iShift ) < 1.e-8 ) iShift = 0.;

    return iShift;
}


double VTargets::getTargetShiftNorth( double ira, double idec )
{
    double degra = atan(1.)/45.;

    double sep  = slaDsep(  getTargetRAJ2000(), getTargetDecJ2000(), ira * degra, idec * degra );
    double bear = slaDbear( getTargetRAJ2000(), getTargetDecJ2000(), ira * degra, idec * degra );

    double iShift = sep * cos( bear ) / degra;

    if( TMath::Abs( iShift ) < 1.e-8 ) iShift = 0.;

    return iShift;
}
